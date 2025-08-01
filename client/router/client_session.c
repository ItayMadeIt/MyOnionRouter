#include <netdb.h>
#include <netinet/in.h>
#include <session.h>

#include <stdlib.h>
#include <sys/epoll.h>
#include <utils/debug.h>

#include "client.h"
#include "net_messages.h"
#include "tor_net_messages.h"
#include "protocol/tor_structs.h"
#include "stream_buffers.h"
#include "stream_hashmap.h"

#include <handle_tls.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MY_EVENT_STDIN 1
#define MY_EVENT_RELAY 2

#define MAX_EVENTS 8

#define INPUT_BUF_SIZE 256
#define TOR_DATA_BUF_SIZE 16

#define MIN(a,b) (a < b ? a : b)


static uint32_t read_input(char *buffer, size_t size, const char *prompt)
{
    printf("%s: ", prompt);

    if (fgets(buffer, size, stdin) != NULL)
    {
        uint32_t newline_index = strcspn(buffer, "\n");
        buffer[newline_index] = '\0'; 
        return newline_index;
    }
    else
    {
        buffer[0] = '\0'; 
        return 0;
    }
}

static uint32_t input_int(const char *prompt)
{
    char input_buff[INPUT_BUF_SIZE];
    read_input(input_buff, INPUT_BUF_SIZE, prompt);
    return atoi(input_buff); 
}

static bool resolve_sockaddr(const char* host, const char* port_str, sock_addr_t* out) 
{
    struct addrinfo hints, *res;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port_str, &hints, &res)) != 0) 
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return false;
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
        return false;
    }

    freeaddrinfo(res);
    return true;
}

static sock_addr_t input_sockaddr()
{
    while (true)
    {
        char ip_addr[INPUT_BUF_SIZE];
        read_input(ip_addr, INPUT_BUF_SIZE, "IP/Addr");

        char port_addr[INPUT_BUF_SIZE];
        read_input(port_addr, INPUT_BUF_SIZE, "Port");

        sock_addr_t sock;
        if (resolve_sockaddr(ip_addr, port_addr, &sock))
        {
            return sock;
        } 

        printf("Invalid sockaddr, try again.\n");
    }
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
    session->cur_relays = 0;
    session->hashmap = NULL;
}

static bool handle_create_message(client_session_t* session, msg_tor_buffer_t* buffer)
{
    if (tor_send_create_msg(session, buffer, &client_vars.id_key) == false)
    {
        return client_error;
    }

    msg_tor_created_t* response = (msg_tor_created_t*)buffer;
    recv_tor_buffer(session->sock_fd, buffer, &session->tls_key, NULL, 0);  
    if (response->cmd != TOR_CREATED)
    {
        return client_error;
    } 
    
    return client_success;
}

client_code_t create_relay_circuit(client_session_t* session, msg_tor_buffer_t* buffer)
{
    while (session->cur_relays < session->relays->relay_amount)
    {
        if (tor_send_extend_msg(session, buffer, &client_vars.id_key, &session->relays->relays[session->cur_relays]) == false)
        {
            return client_error;
        }

        msg_tor_relay_extended_t* extended = (msg_tor_relay_extended_t*)buffer;
        if (recv_tor_buffer(
            session->sock_fd, buffer, &session->tls_key, session->onion_keys, session->cur_relays) == false)
        {
            return client_error;
        }

        if (extended->cmd != RELAY_EXTENDED)
        {
            return client_error;
        }

        session->cur_relays++;
    }

    return client_success;
}

inline static bool handle_relay_relay_msg(client_session_t* session, msg_tor_buffer_t* buffer, client_code_t* response)
{
    (void)session; (void)response;
    
    msg_tor_relay_t* relay_buffer = (msg_tor_relay_t*)buffer;

    if (relay_buffer->cmd == RELAY_CONNECTED)
    {
        printf("[RELAY] Recieved connnected: %d\n", relay_buffer->stream_id);

        return true;
    }

    if (relay_buffer->cmd == RELAY_END)
    {
        printf("[RELAY] Disconnected from: %d\n", relay_buffer->stream_id);

        socket_hashmap_remove(session->hashmap, relay_buffer->stream_id);

        return true;
    }

    if (relay_buffer->cmd == RELAY_DATA)
    {
        printf("[RELAY] Get data: %d\n", relay_buffer->stream_id);

        stream_hashmap_entry_t* entry =
            socket_hashmap_find(session->hashmap, relay_buffer->stream_id);

        if (entry == NULL)
        {
            printf("[ERROR] GOT DATA TO INVALID STREAM ID\n");
            return false;
        }

        stream_push_data(&entry->data.recv_buffer, relay_buffer->data, relay_buffer->length);

        return true;
    }

    return  true;
}

bool handle_relay_msg(client_session_t* session, msg_tor_buffer_t* buffer, client_code_t* response)
{
    (void)session; (void)response;

    msg_tor_t* msg_buffer = (msg_tor_t*)buffer;
    
    if (msg_buffer->cmd == TOR_RELAY)
    {
        return handle_relay_relay_msg(session, buffer, response);
    }

    if (msg_buffer->cmd == TOR_PADDING)
    {
        printf("[CMD] Recieved padding.\n");
        return true;
    }

    return true;
}

inline static bool handle_input_ping(client_session_t* session, msg_tor_buffer_t* msg, client_code_t* response)
{
    if (tor_send_ping_msg(session, msg) == false)
    {
        fprintf(stderr, "Failed send buffer.\n");
        *response = client_error;
        return false;
    }
    
    return true;
}

inline static bool handle_input_destroy(client_session_t* session, msg_tor_buffer_t* msg, client_code_t* response)
{
    if (tor_send_destroy_msg(session, msg) == false)
    {
        fprintf(stderr, "Failed send buffer.\n");
        *response = client_error;
        return false;
    }

    *response = client_success;
    return false;
}

inline static bool handle_input_connect(client_session_t* session, msg_tor_buffer_t* msg, client_code_t* response)
{
    sock_addr_t sock = input_sockaddr();

    uint32_t stream_id = gen_stream_id(session->hashmap);
    socket_hashmap_insert(session->hashmap, stream_id);
    printf("Stream ID: %u\n", stream_id);
    
    if (tor_send_begin_msg(session, msg, stream_id, &sock) == false)
    {
        fprintf(stderr, "Failed send buffer.\n");
        *response = client_error;
        return false;
    }

    return true;
}

inline static bool handle_input_close(client_session_t* session, msg_tor_buffer_t* msg, client_code_t* response)
{
    uint32_t stream_id = input_int("Stream ID");
    
    if (tor_send_end_msg(session, msg, stream_id) == false)
    {
        fprintf(stderr, "Failed send buffer.\n");
        *response = client_error;
        return false;
    }

    socket_hashmap_remove(session->hashmap, stream_id);

    return true;
}

inline static bool handle_input_send(client_session_t* session, msg_tor_buffer_t* msg, client_code_t* response)
{
    uint32_t stream_id = input_int("Stream ID");

    stream_hashmap_entry_t* entry = socket_hashmap_find(session->hashmap, stream_id);
    if (entry == NULL)
    {
        printf("Couldn't find stream ID.\n");
        return true;
    }

    uint32_t length = input_int("Data length");
    char* data_input_buffer = (char*)malloc(sizeof(char) * length);
    uint32_t input_length = read_input(data_input_buffer, length, "Data");

    uint32_t index = 0;
    while (index < input_length)
    {
        uint32_t cur_data_length = MIN(RELAY_DATA_MSG_SIZE, input_length - index);

        if (tor_send_data_msg(
            session, msg, stream_id, (uint8_t*)data_input_buffer + index, cur_data_length) == false)
        {
            fprintf(stderr, "Failed send buffer.\n");
            
            free(data_input_buffer);

            *response = client_error;
            return false;
        }

        index += cur_data_length;
    }

    free(data_input_buffer);

    return true;
}

inline static bool handle_input_recv(client_session_t* session, msg_tor_buffer_t* msg, client_code_t* response)
{
    (void)session; (void)msg; (void)response;

    uint32_t stream_id = input_int("Stream ID");

    stream_hashmap_entry_t* entry = socket_hashmap_find(session->hashmap, stream_id);
    if (entry == NULL)
    {
        printf("Couldn't find stream ID.\n");
        return true;
    }

    uint32_t length = input_int("Length");
    
    char buffer[TOR_DATA_BUF_SIZE+1];

    while (length)
    {
        uint32_t amount = stream_pop_data(&entry->data.recv_buffer, (uint8_t*)buffer, MIN(TOR_DATA_BUF_SIZE, length));

        if (amount == 0)
        {
            break;
        }

        if (amount <= length)
        {
            length -= amount;
            buffer[amount] = '\0';
            printf("%s", buffer);
            continue;
        }
    }

    printf("\n");

    return true;
}

bool handle_input(client_session_t* session, const char input_buffer[INPUT_BUF_SIZE], client_code_t* response)
{
    if (strcmp(input_buffer, "exit") == 0)
    {
        *response = client_success;
        return false;
    }

    msg_tor_buffer_t msg;

    if (strcmp(input_buffer, "ping") == 0)
    {
        return handle_input_ping(session, &msg, response);
    }
    else if (strcmp(input_buffer, "destroy") == 0)
    {
        return handle_input_destroy(session, &msg, response);
    }
    else if (strcmp(input_buffer, "connect") == 0)
    {
        return handle_input_connect(session, &msg, response);
    }
    else if (strcmp(input_buffer, "close") == 0)
    {
        return handle_input_close(session, &msg, response);
    }
    else if (strcmp(input_buffer, "send") == 0)
    {
        return handle_input_send(session, &msg, response);
    }
    else if (strcmp(input_buffer, "recv") == 0)
    {
        return handle_input_recv(session, &msg, response);
    }

    printf("Invalid command: %s\n", input_buffer);
    return true;
}

bool handle_event(client_session_t* session, msg_tor_buffer_t* buffer, struct epoll_event* event, client_code_t* response)
{
    if (event->data.u32 == MY_EVENT_RELAY)
    {
        if (recv_tor_buffer(session->sock_fd, buffer, &session->tls_key, session->onion_keys, session->cur_relays) == false)
        {
            fprintf(stderr, "Guard stopped connection.\n");
            *response = client_error;
            return false;
        }

        if (handle_relay_msg(session, buffer, response) == false)
        {
            return false;
        }
    }
    else if (event->data.u32 == MY_EVENT_STDIN)
    {
        char input_buffer[INPUT_BUF_SIZE];
        fgets(input_buffer, INPUT_BUF_SIZE, stdin);
        input_buffer[strcspn(input_buffer, "\n")] = '\0';

        printf("Command: '%s'\n", input_buffer);

        if (handle_input(session, input_buffer, response) == false)
        {
            return false;
        }
    }
    else
    {
        fprintf(stderr, "Invalid u32, shouldn't happen: %d\n", event->data.u32);
        abort();
    }
    return true;
}

bool handle_event_chunk(client_session_t* session, int epoll_fd, msg_tor_buffer_t* buffer, client_code_t* response)
{
    struct epoll_event events[MAX_EVENTS];
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    
    if (n == -1)
    {
        *response = client_error;

        return false;
    }

    for (int i = 0; i < n; i++)
    {
        if (handle_event(session, buffer, &events[i], response) == false)
        {
            return false;
        }
    }
    
    return true;
}
client_code_t process_events(client_session_t* session, msg_tor_buffer_t* buffer)
{
    client_code_t response = client_success; 

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) 
    {
        fprintf(stderr, "Couldn't create an epoll: epoll_create1\n");
        exit(-1);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.u32 = MY_EVENT_STDIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fileno(stdin), &ev);

    ev.data.u32 = MY_EVENT_RELAY;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, session->sock_fd, &ev);

    while (true)
    {
        if (handle_event_chunk(session, epoll_fd, buffer, &response) == false)
        {
            close(epoll_fd);
            socket_hashmap_free(session->hashmap);
            session->hashmap = NULL;

            return response;
        }
    }

    close(epoll_fd);
    socket_hashmap_free(session->hashmap);
    session->hashmap = NULL;
    
    return client_success;
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
        free_session(session);
        return return_value;
    }

    printf("Circuit has been created.\n");

    stream_hashmap_t hashmap;
    socket_hashmap_init(&hashmap);
    session->hashmap = &hashmap;

    return process_events(session, &buffer);
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