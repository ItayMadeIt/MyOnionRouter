
#include "net_messages.h"
#include "protocol/relay_data_structs.h"
#include "protocol/tor_structs.h"
#include "handle_tls.h"
#include "relay.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "session.h"
#include <utils/sock_utils.h>
#include <netinet/in.h>

static void sockaddr_to_unix(struct sockaddr_storage* storage, socklen_t* storage_length, const sock_addr_t* sock_addr)
{
    memset(storage, 0, sizeof(struct sockaddr_storage));
    
    if (sock_addr->family == AF_INET) 
    {
        struct sockaddr_in* addr4 = (struct sockaddr_in*)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = sock_addr->port;  // make sure it's in network byte order

        memcpy(&addr4->sin_addr, sock_addr->addr, IP4_SIZE);

        *storage_length = sizeof(struct sockaddr_in);
    } 
    else if (sock_addr->family == AF_INET6) 
    {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = sock_addr->port;
        memcpy(&addr6->sin6_addr, sock_addr->addr, IP6_SIZE);

        *storage_length = sizeof(struct sockaddr_in6);
    }

    else 
    {
        // Unsupported family
        memset(storage, 0, sizeof(struct sockaddr_storage));
        *storage_length = 0;
    }
}

static relay_code_t extend_relay(relay_session_t* session, msg_tor_buffer_t* buffer)
{
    msg_tor_relay_extend_t* extend = (msg_tor_relay_extend_t*)buffer;
    
    struct sockaddr_storage relay_addr;
    socklen_t length;
    sockaddr_to_unix(&relay_addr, &length, &extend->relay_addr);
    session->next_fd = connect_server_by_sockaddr(&relay_addr, length);

    if (handle_tls_sender(session->next_fd, &session->tls_next_key) == false)
    {
        printf("Failed TLS\n");
        free_relay_session(session);
        return relay_error;
    }

    printf("TLS worked\n");
    msg_tor_create_t* create = (msg_tor_create_t*)buffer;
    create->cmd = TOR_CREATE;
    memmove(create->public_client_key, extend->client_public_key, ASYMMETRIC_KEY_BYTES);
    // Send CREATE msg
    if (send_tor_buffer(session->next_fd, buffer, &session->tls_next_key, NULL, false) == false)
    {
        free_relay_session(session);
        return relay_error;
    }

    msg_tor_created_t* created = (msg_tor_created_t*)buffer;
    // Recv CREATED msg
    if (recv_tor_buffer(session->next_fd, buffer, &session->tls_next_key, NULL, false) == false)
    {
        free_relay_session(session);
        return relay_error;
    }
    if (created->cmd != TOR_CREATED)
    {
        printf("Failed to get CREATED\n");
    }
    printf("Will send extended.\n");

    msg_tor_relay_extended_t* extended = (msg_tor_relay_extended_t*)buffer;
    extended->relay = TOR_RELAY;
    extended->cmd = RELAY_EXTENDED;

    if (send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
    {
        free_relay_session(session);
        return relay_error;
    }
    
    return relay_success;
}

relay_code_t process_middle_relay_session(relay_session_t* session, msg_tor_buffer_t* buffer)
{
    relay_code_t response = relay_success;
    response = extend_relay(session, buffer);
    if (response != relay_success)
    {
        free_relay_session(session);
        return response;
    }

    printf("Sent extended relay");

    bool from_client = true;

    // Sample recv loop (recv from client then from internet in a loop)
    while (true)
    {
        int recv_sock_fd = from_client ? session->last_fd : session->next_fd;
        key_data_t* recv_tls_key = from_client ? &session->tls_last_key : &session->tls_next_key;

        if (recv_tor_buffer(recv_sock_fd, buffer, recv_tls_key, &session->onion_key, from_client) == false)
        {
            free_relay_session(session);
            return relay_error;
        }
        printf("RECV {FROM CLIENT}:%d \n", from_client);

        int send_sock_fd = from_client ? session->next_fd : session->last_fd;
        key_data_t* send_tls_key = from_client ? &session->tls_next_key : &session->tls_last_key;

        if (send_tor_buffer(send_sock_fd, buffer, send_tls_key, &session->onion_key, !from_client/*To the other way*/) == false)
        {
            free_relay_session(session);
            return relay_error;
        }
        printf("SEND {FROM CLIENT}:%d \n", from_client);

        from_client = !from_client;
    }

    return relay_success;
}