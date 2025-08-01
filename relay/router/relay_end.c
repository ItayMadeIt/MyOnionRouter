#include "net_messages.h"
#include "protocol/tor_structs.h"
#include "session.h"
#include "relay.h"
#include "socket_hashmap.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <utils/sock_utils.h>
#include "relay_messages.h"

#define MAX_EVENTS 1 // only 1 event at a time
#define BUFFER_SIZE 128

#define CLIENT_STREAM_ID (uint32_t)(~0)

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

static ssize_t read_buffer(int sock_fd, uint8_t* buffer, const uint32_t buffer_size)
{
    while (true)
    {
        ssize_t amount = read(sock_fd, buffer, buffer_size);

        if (amount >= 0)
        {
            return amount;
        }
        
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return 0;
        }
        if (errno == EINTR)
        {
            continue;
        }

        return -1;
   }
}

static bool handle_relay_begin(relay_session_t* session, socket_hashmap_t* hashmap, /*remove const*/ int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    msg_tor_relay_begin_t* msg_begin = (msg_tor_relay_begin_t*)buffer;
    
    struct sockaddr_storage relay_addr;
    socklen_t length;
    sockaddr_to_unix(&relay_addr, &length, &msg_begin->sock_addr);
    int sock_fd = connect_server_by_sockaddr(&relay_addr, length);

    uint16_t stream_id = msg_begin->stream_id;

    if (sock_fd == -1)
    {
        if (tor_send_end_relay_end(session, buffer, stream_id, TOR_REASON_END_FAIL_INIT) == false)
        {
            *response = relay_error;
            return false;
        }

        return true;
    }

    socket_hashmap_entry_t* entry =
        socket_hashmap_insert(hashmap, stream_id, sock_fd);

    if (entry == NULL)
    {
        if (tor_send_end_relay_end(session, buffer, stream_id, TOR_REASON_END_FAIL_INIT) == false)
        {
            *response = relay_error;
            return false;
        }

        return true;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.u32 = entry->data.stream_id;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev);

    if (tor_send_end_relay_connected(session, buffer, stream_id) == false)
    {
        *response = relay_error;
        return false;
    }

    return true;
}

static bool handle_relay_data(relay_session_t* session, socket_hashmap_t* hashmap, /*remove const*/ int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    (void)epoll_fd;

    msg_tor_relay_data_t* msg_data = (msg_tor_relay_data_t*)buffer;

    uint32_t stream_id = msg_data->stream_id;
    socket_hashmap_entry_t* entry = socket_hashmap_find(hashmap, stream_id);

    if (entry == NULL)
    {
        if (tor_send_end_relay_end(session, buffer, stream_id, TOR_REASON_END_ABSENT) == false)
        {
            *response = relay_error;
            return false;
        }

        return true;
    }

    int sock_fd = entry->data.sock_fd;

    if (send(sock_fd, msg_data->data, msg_data->length, 0) <= 0)
    {
        if (tor_send_end_relay_end(session, buffer, stream_id, TOR_REASON_END_EOF) == false)
        {
            *response = relay_error;
            return false;
        }

        return true;
    }

    return true;
}

static bool handle_relay_end(relay_session_t* session, socket_hashmap_t* hashmap, /*remove const*/ int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    (void)session; (void)response;

    msg_tor_relay_end_t* msg_end = (msg_tor_relay_end_t*)buffer;

    uint32_t stream_id = msg_end->stream_id;
    socket_hashmap_entry_t* entry = socket_hashmap_find(hashmap, stream_id);

    if (entry == NULL)
    {
        if (tor_send_end_relay_end(session, buffer, stream_id, TOR_REASON_END_ABSENT) == false)
        {
            socket_hashmap_free(hashmap);            
            *response = relay_error;
            return false;
        }

        return true;
    }

    int sock_fd = entry->data.sock_fd;

    struct epoll_event ev;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock_fd, &ev);

    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);

    socket_hashmap_remove(hashmap, stream_id);

    if (tor_send_end_relay_end(session, buffer, stream_id, TOR_REASON_END_DONE) == false)
    {
        *response = relay_error;

        return false;
    }
    
    return true;
}

static bool handle_client_msg(relay_session_t* session, socket_hashmap_t* hashmap, /*remove const*/ int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    msg_tor_t* msg = (msg_tor_t*)buffer; 
    if (msg->cmd == TOR_DESTROY)
    {
        *response = relay_success;
        return false;
    } 

    if (msg->cmd == TOR_PADDING)
    {
        if (send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
        {
            *response = relay_error;
            return false;
        }

        return true;
    }

    // A message we dont yet handle.
    if (msg->cmd != TOR_RELAY)
    {
        printf("NOT SUPPORTING CMD %u\n", msg->cmd);
        
        return true;
    }

    msg_tor_relay_t* relay_msg = (msg_tor_relay_t*)buffer;

    switch (relay_msg->cmd)
    { 
        case RELAY_BEGIN:
        {
            return handle_relay_begin(session, hashmap, epoll_fd, buffer, response);
        }
        case RELAY_END:
        {
            return handle_relay_end(session, hashmap, epoll_fd, buffer, response);
        }
        case RELAY_DATA:
        {
            return handle_relay_data(session, hashmap, epoll_fd, buffer, response);
        }
        default:
        {
            printf("NOT SUPPORTING RELAY CMD %u\n", msg->cmd);
        }
    }
    
    return true;
}

static bool handle_internet_event(stream_data_t* stream_data, relay_session_t* session, socket_hashmap_t* hashmap, const int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    (void)response;

    uint8_t recv_buffer_data[RELAY_DATA_MSG_SIZE];
    ssize_t size = read_buffer(stream_data->sock_fd, recv_buffer_data, RELAY_DATA_MSG_SIZE);

    uint16_t stream_id = stream_data->stream_id;

    if (size <= 0)
    {
        struct epoll_event ev;
        ev.events = EPOLLIN;
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL,stream_data->sock_fd, &ev);
 
        close(stream_data->sock_fd);
        socket_hashmap_remove(hashmap, stream_id);
        
        if (tor_send_end_relay_end(session, buffer, stream_id, TOR_REASON_END_EOF) == false)
        {
            *response = relay_error;

            return false;
        }

        return true;
    }

    if (tor_send_end_relay_data(session, buffer, stream_id, recv_buffer_data, size) == false)
    {
        *response = relay_error;

        return false;
    }
    
    return true;
} 

static bool handle_event(relay_session_t* session, struct epoll_event* event, socket_hashmap_t* hashmap, const int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    if (event->data.u32 == CLIENT_STREAM_ID)
    {
        if (recv_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
        {
            *response = relay_error;

            return false;
        }

        return handle_client_msg(session, hashmap, epoll_fd, buffer, response);
    }
    else
    {
        socket_hashmap_entry_t* entry = socket_hashmap_find(hashmap, event->data.u32);
        
        if (entry == NULL)
        {
            *response = relay_error;
            return false;
        }
        
        return handle_internet_event(&entry->data, session, hashmap, epoll_fd, buffer, response);
    }
}

static bool handle_epoll_event(const int epoll_fd, relay_session_t* session, socket_hashmap_t* hashmap, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    struct epoll_event event;
    int n = epoll_wait(epoll_fd, &event, MAX_EVENTS, -1);

    if (n == -1)
    {
        if (errno == EINTR)
        {    
            return true;
        }

        *response = relay_error;

        return false;
    }

    return handle_event(session, &event, hashmap, epoll_fd, buffer, response);
}


relay_code_t process_end_relay_session(relay_session_t* session, msg_tor_buffer_t* buffer)
{
    relay_code_t response = relay_success;

    socket_hashmap_t hashmap;
    socket_hashmap_init(&hashmap);

    int epoll_fd = epoll_create1(0);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.u32 = CLIENT_STREAM_ID;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, session->last_fd, &ev);

    if (!handle_client_msg(session, &hashmap, epoll_fd, buffer, &response))
    {
        return response;
    }

    while (true)
    {
        if (handle_epoll_event(epoll_fd, session, &hashmap, buffer, &response) == false)
        {
            close(epoll_fd);
            socket_hashmap_free(&hashmap);

            return response;
        }
    }
}