#include <netdb.h>
#include <netinet/in.h>
#include <session.h>

#include <stdlib.h>
#include <sys/epoll.h>
#include <utils/debug.h>

#include "client.h"
#include "net_messages.h"
#include "protocol/tor_structs.h"
#include "stream_hashmap.h"

#include <handle_tls.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MY_EVENT_STDIN 1
#define MY_EVENT_RELAY 2

#define MAX_EVENTS 8

#define INPUT_BUF_SIZE 256


int resolve_sockaddr(const char* host, const char* port_str, sock_addr_t* out) 
{
    struct addrinfo hints, *res;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port_str, &hints, &res)) != 0) 
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

    if (res->ai_family == AF_INET) 
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
        out->family = AF_INET;
        out->protocol = SOCK_STREAM;
        memcpy(out->addr, &(ipv4->sin_addr), IP4_SIZE);
        out->port = ipv4->sin_port;
    } 
    else if (res->ai_family == AF_INET6) 
    {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
        out->family = AF_INET6;
        out->protocol = SOCK_STREAM;
        memcpy(out->addr, &(ipv6->sin6_addr), IP6_SIZE);
        out->port = ipv6->sin6_port;
    } 
    else 
    {
        fprintf(stderr, "Unsupported address family.\n");
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return 0;
}

void init_session(client_session_t* session, int sock_fd, const circuit_relay_list_t* relay_list)
{
    session->sock_fd = sock_fd;
    session->config = client_vars.config;
    for (uint8_t i = 0; i < relay_list->relay_amount; i++)
    {
        init_key(&session->onion_keys[i], &client_vars.id_key);
        derive_symmetric_key_from_public(&session->onion_keys[i], relay_list->relays[i].public_key);
    }
    init_key(&session->tls_key, &client_vars.id_key);
}

static bool handle_create_message(client_session_t* session, msg_tor_buffer_t* buffer)
{
    // Send CREATE msg
    msg_tor_create_t* create_msg = (msg_tor_create_t*)buffer;
    create_msg->circ_id = 0;
    create_msg->cmd = TOR_CREATE;
    get_public_identity_key(&client_vars.id_key, create_msg->public_client_key);

    send_tor_buffer(session->sock_fd, buffer, &session->tls_key, NULL, 0);

    // Send CREATE msg
    msg_tor_created_t* response = (msg_tor_created_t*)buffer;
    recv_tor_buffer(session->sock_fd, buffer, &session->tls_key, NULL, 0);
    
    if (response->cmd != TOR_CREATED)
    {
        free_session(session);
        return client_error;
    } 
    
    return client_success;
}

client_code_t create_relay_circuit(client_session_t* session, msg_tor_buffer_t* buffer)
{
    while (session->cur_relays < session->relays->relay_amount)
    {
        msg_tor_relay_extend_t* msg_extend = (msg_tor_relay_extend_t*)buffer;
        msg_extend->relay = TOR_RELAY;

        msg_extend->cmd = RELAY_EXTEND;
        msg_extend->circ_id = 0;
        memset(&msg_extend->digest, 0, DIGEST_LEN);
        
        get_public_identity_key(session->tls_key.identity, msg_extend->client_public_key);
        
        memcpy(
            &msg_extend->relay_addr, 
            &session->relays->relays[session->cur_relays], 
            sizeof(sock_addr_t)
        );

        // session->onion_keys
        if (!send_tor_buffer(
            session->sock_fd, buffer,  &session->tls_key, session->onion_keys, session->cur_relays))
        {
            close(session->sock_fd);
            return client_error;
        }

        if (!recv_tor_buffer(
            session->sock_fd, buffer, &session->tls_key, session->onion_keys, session->cur_relays))
        {
            close(session->sock_fd);
            return client_error;            
        }

        msg_tor_relay_extended_t* extended = (msg_tor_relay_extended_t*)buffer;
        if (extended->cmd != RELAY_EXTENDED)
        {
            close(session->sock_fd);
            return client_error;
        }

        session->cur_relays++;
    }

    return client_success;
}

bool handle_input(client_session_t* session, stream_hashmap_t* hashmap, const char input_buffer[INPUT_BUF_SIZE])
{
    msg_tor_buffer_t msg;

    if (strcmp(input_buffer, "ping") == 0)
    {
        msg_tor_padding_t* padding = (msg_tor_padding_t*) &msg;
        padding->circ_id = 0;
        padding->cmd = TOR_PADDING;

        if (send_tor_buffer(session->sock_fd, &msg, &session->tls_key, session->onion_keys, session->cur_relays) == false)
        {
            close(session->sock_fd);
            socket_hashmap_free(hashmap);
            return false;
        }
        return true;
    }
    else if (strcmp(input_buffer, "connect") == 0)
    {
        printf("IP/Addr: ");

        char ip_addr[INPUT_BUF_SIZE];
        fgets(ip_addr, INPUT_BUF_SIZE, stdin);
        ip_addr[strcspn(ip_addr, "\n")] = '\0';

        printf("Port: ");

        char port_addr[INPUT_BUF_SIZE];
        fgets(port_addr, INPUT_BUF_SIZE, stdin);
        port_addr[strcspn(port_addr, "\n")] = '\0';

        uint32_t stream_id = gen_stream_id(hashmap);        
        socket_hashmap_insert(hashmap, stream_id);
        printf("Stream ID: %u\n", stream_id);

        msg_tor_relay_begin_t* begin = (msg_tor_relay_begin_t*)&msg;
        begin->relay = TOR_RELAY;
        begin->cmd = RELAY_BEGIN;
        begin->stream_id = stream_id;
        resolve_sockaddr(ip_addr, port_addr, &begin->sock_addr); 

        if (send_tor_buffer(session->sock_fd, &msg, &session->tls_key, session->onion_keys, session->cur_relays) == false)
        {
            printf("Failed send buffer.\n");
            close(session->sock_fd);
            socket_hashmap_free(hashmap);
            return false;
        }

        return true;
    }

    printf("Invalid command: %s\n", input_buffer);
    return true;
}

client_code_t process_client_session(client_session_t* session)
{
    client_code_t return_value = client_success;

    msg_tor_buffer_t buffer = {0};
    return_value = handle_create_message(session, &buffer);
    if (return_value != client_success)
    {
        free_session(session);
        return return_value;
    }


    session->cur_relays = 1;
    return_value = create_relay_circuit(session, &buffer);
    if (return_value != client_success)
    {
        session->sock_fd = -1;
        free_session(session);
        return return_value;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) 
    {
        fprintf(stderr, "Couldn't create an epoll: epoll_create1\n");
        exit(-1);
    }

    stream_hashmap_t hashmap;
    socket_hashmap_init(&hashmap);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.u32 = MY_EVENT_STDIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fileno(stdin), &ev);

    ev.data.u32 = MY_EVENT_RELAY;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, session->sock_fd, &ev);

    while (true)
    {
        struct epoll_event events[MAX_EVENTS];
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        
        if (n == -1)
        {
            socket_hashmap_free(&hashmap);
            return client_error;
        }
        int i = 0;
        while (i < n)
        {
            if (events[i].data.u32 == MY_EVENT_RELAY)
            {

            }
            else if (events[i].data.u32 == MY_EVENT_STDIN)
            {
                char buffer[256];
                if (fgets(buffer, INPUT_BUF_SIZE, stdin) == NULL) 
                {
                    fprintf(stderr, "stdin closed or read error\n");
                    exit(-1);
                }
                buffer[strcspn(buffer, "\n")] = '\0';

                printf("Command: '%s' (len: %zu)\n", buffer, strlen(buffer));

                if (strcmp(buffer, "exit") == 0)
                {
                    close(session->sock_fd);
                    socket_hashmap_free(&hashmap);

                    return client_success;
                }

                if (handle_input(session, &hashmap, buffer) == false)
                {
                    close(session->sock_fd);
                    socket_hashmap_free(&hashmap);
                    return client_error;
                }
            }
            else
            {
                fprintf(stderr, "Invalid u32, shouldn't happen: %d\n", ev.data.u32);
                abort();
            }
            i++;
        }
    }

    close(session->sock_fd);
    socket_hashmap_free(&hashmap);

    return client_success;
}

void free_session(client_session_t* session)
{
    if (session->sock_fd > 0)
    {
        close(session->sock_fd);
        session->sock_fd = -1;
    }
    free_key(&session->tls_key);
    for (uint8_t i = 0; i < session->config->relays; i++)
    {
        free_key(&session->onion_keys[i]);
    }
}