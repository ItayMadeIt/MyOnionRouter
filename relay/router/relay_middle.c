#include "relay.h"

#include "net_messages.h"
#include "protocol/relay_data_structs.h"
#include "protocol/tor_structs.h"
#include "handle_tls.h"
#include <stdio.h>
#include <string.h>
#include "session.h"
#include "relay_messages.h"

#include <utils/sock_utils.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 256
#define MAX_EVENTS 1 // only 1 event at a time

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
        return relay_error;
    }

    if (tor_send_middle_relay_create(session, buffer, extend->client_public_key) == false)
    {
        return relay_error;
    }

    msg_tor_created_t* created = (msg_tor_created_t*)buffer;
    if (recv_tor_buffer(session->next_fd, buffer, &session->tls_next_key, NULL, false) == false)
    {
        return relay_error;
    }
    if (created->cmd != TOR_CREATED)
    {
        return relay_error;
    }

    if (tor_send_middle_relay_extended(session, buffer) == false)
    {
        return relay_error;
    }
    
    return relay_success;
}

static bool handle_fd_common(relay_session_t* session, bool from_client, relay_code_t* response, msg_tor_t* recv_msg, msg_tor_t* send_msg_buffer)
{
    int recv_sock_fd = from_client ? session->last_fd : session->next_fd;
    key_data_t* recv_tls_key = from_client ? &session->tls_last_key : &session->tls_next_key;

    if (recv_tor_buffer(recv_sock_fd, (msg_tor_buffer_t*)recv_msg, recv_tls_key, &session->onion_key, from_client) == false)
    {
        *response = relay_error;
        return false;
    }
    memmove(send_msg_buffer, recv_msg, sizeof(msg_tor_buffer_t));

    int send_sock_fd = from_client ? session->next_fd : session->last_fd;
    key_data_t* send_tls_key = from_client ? &session->tls_next_key : &session->tls_last_key;

    if (send_tor_buffer(send_sock_fd, (msg_tor_buffer_t*)send_msg_buffer, send_tls_key, &session->onion_key, !from_client) == false)
    {
        *response = relay_error;
        return false;
    }

    return true;
}

static bool handle_last_fd(relay_session_t* session, relay_code_t* response)
{
    msg_tor_t recv, send;
    if (handle_fd_common(session, true, response, &recv, &send) == false)
    {
        return false;
    }

    if (recv.cmd == TOR_DESTROY)
    {
        *response = relay_success;
        return false;
    }
    
    return true;
}

static bool handle_next_fd(relay_session_t* session, relay_code_t* response)
{
    msg_tor_t recv;
    msg_tor_t send;

    if (handle_fd_common(session, false, response, &recv, &send) == false)
    {
        return false;
    }

    return true;
}


static bool process_event(relay_session_t* session, struct epoll_event* event, relay_code_t* response)
{
    if (event->data.fd == session->last_fd)
    {
        return handle_last_fd(session, response);
    }
    else
    {
        return handle_next_fd(session, response);
    }
}

relay_code_t process_middle_relay_session(relay_session_t* session, msg_tor_buffer_t* buffer)
{
    relay_code_t response = relay_success;
    response = extend_relay(session, buffer);
    if (response != relay_success)
    {
        return response;
    }

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev, event;
   
    ev.events = EPOLLIN;
    ev.data.fd = session->last_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, session->last_fd, &ev) == -1)
    {
        close(epoll_fd);
        return relay_error;
    }

    ev.events = EPOLLIN;
    ev.data.fd = session->next_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, session->next_fd, &ev) == -1)
    {
        close(epoll_fd);
        return relay_error;
    }

    while (true)
    {
        int events_amount = epoll_wait(epoll_fd, &event, MAX_EVENTS, -1);

        if (events_amount == -1)
        {
            close(epoll_fd);
            return relay_error;
        }

        if (process_event(session, &event, &response) == false)
        {
            return response;
        }
    }
    
    // Will never get to here
    close(epoll_fd);
    return relay_success;
}