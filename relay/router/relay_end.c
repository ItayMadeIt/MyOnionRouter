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

static bool process_relay_begin(relay_session_t* session, socket_hashmap_t* hashmap, /*remove const*/ int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    msg_tor_relay_begin_t* msg_begin = (msg_tor_relay_begin_t*)buffer;
    
    struct sockaddr_storage relay_addr;
    socklen_t length;
    sockaddr_to_unix(&relay_addr, &length, &msg_begin->sock_addr);
    int sock_fd = connect_server_by_sockaddr(&relay_addr, length);

    if (sock_fd == -1)
    {
        msg_tor_relay_end_t* msg_end = (msg_tor_relay_end_t*)buffer;
        msg_end->cmd = RELAY_END;
        msg_end->reason = TOR_REASON_END_EOF;
        if (send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
        {
            close(epoll_fd);
            free_relay_session(session);
            socket_hashmap_free(hashmap);
            *response = relay_error;
            return false;
        }

        return true;
    }

    socket_hashmap_entry_t* entry =
        socket_hashmap_insert(hashmap, msg_begin->stream_id, sock_fd);

    if (entry == NULL)
    {
        fprintf(stderr, "Invalid stream id, already used.\n");
        return true;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.u32 = entry->data.stream_id;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev);

    printf("Added a new connection!\n");

    msg_tor_relay_connected* msg_connected = (msg_tor_relay_connected*)buffer;
    msg_connected->relay = TOR_RELAY;
    msg_connected->cmd = RELAY_CONNECTED;
    if (send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
    {
        close(epoll_fd);
        free_relay_session(session);
        socket_hashmap_free(hashmap);
        *response = relay_error;
        return false;
    }

    return true;
}

static bool process_relay_data(relay_session_t* session, socket_hashmap_t* hashmap, /*remove const*/ int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    msg_tor_relay_data_t* msg_data = (msg_tor_relay_data_t*)buffer;

    uint32_t stream_id = msg_data->stream_id;
    socket_hashmap_entry_t* entry = socket_hashmap_find(hashmap, stream_id);

    if (entry == NULL)
    {
        fprintf(stderr, "[ERROR] The client sent an invalid stream id with no valid connection.");

        return true;
    }

    int sock_fd = entry->data.sock_fd;

    if (send(sock_fd, msg_data->data, msg_data->length, 0) <= 0)
    {
        fprintf(stderr, "[ERROR] Failed socket `send`.");

        // Send error to client about socket stream failure
        msg_tor_relay_end_t* msg_end = (msg_tor_relay_end_t*)buffer;
        msg_end->cmd = RELAY_END;
        msg_end->reason = TOR_REASON_END_EOF;
        if (send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
        {
            close(epoll_fd);
            free_relay_session(session);
            socket_hashmap_free(hashmap);
            *response = relay_error;
            return false;
        }

        return true;
    }

    return true;
}

static bool process_relay_end(relay_session_t* session, socket_hashmap_t* hashmap, /*remove const*/ int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    (void)session; (void)response;

    msg_tor_relay_end_t* msg_end = (msg_tor_relay_end_t*)buffer;

    uint32_t stream_id = msg_end->stream_id;
    socket_hashmap_entry_t* entry = socket_hashmap_find(hashmap, stream_id);

    if (entry == NULL)
    {
        fprintf(stderr, "[ERROR] The client sent an invalid stream id with no valid connection.");

        msg_end->cmd = RELAY_END;
        msg_end->reason = TOR_REASON_END_EOF;
        if (send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
        {
            close(epoll_fd);
            free_relay_session(session);
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

    msg_end->cmd = RELAY_END;
    msg_end->reason = TOR_REASON_END_DONE;
    msg_end->stream_id = stream_id;
    printf("Sending msg end %u.\n", stream_id);
    if (send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
    {
        close(epoll_fd);
        free_relay_session(session);
        socket_hashmap_free(hashmap);
        
        *response = relay_error;

        return false;
    }
    
    return true;
}

static bool process_client_msg(relay_session_t* session, socket_hashmap_t* hashmap, /*remove const*/ int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    msg_tor_t* msg = (msg_tor_t*)buffer; 
    if (msg->cmd == TOR_DESTROY)
    {
        printf("Destroy\n");
        close(epoll_fd);
        free_relay_session(session);
        socket_hashmap_free(hashmap);
        *response = relay_success;
        return false;
    } 

    if (msg->cmd == TOR_PADDING)
    {
        printf("Padding\n");
        send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true);
        return true;
    }

    // A message we dont yet handle.
    if (msg->cmd != TOR_RELAY)
    {
        printf("NOT SUPPORTING CMD %u\n", msg->cmd);
        
        return false;// MUST BE CHANGED
    }

    msg_tor_relay_t* relay_msg = (msg_tor_relay_t*)buffer;

    printf("Relay cmd: %u \n", relay_msg->cmd);

    if (relay_msg->cmd == RELAY_BEGIN)
    {
        return process_relay_begin(session, hashmap, epoll_fd, buffer, response);
    }
    else if (relay_msg->cmd == RELAY_END)
    {
        return process_relay_end(session, hashmap, epoll_fd, buffer, response);
    }
    else if (relay_msg->cmd == RELAY_DATA)
    {
        return process_relay_data(session, hashmap, epoll_fd, buffer, response);
    }

    return true;
}

static bool process_internet_event(stream_data_t* stream_data, relay_session_t* session, socket_hashmap_t* hashmap, const int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    (void)response;

    msg_tor_relay_t* tor_msg = (msg_tor_relay_t*)buffer;

    ssize_t size = read_buffer(stream_data->sock_fd, tor_msg->data, RELAY_MSG_SIZE);
    if (size <= 0)
    {
        printf("Failed to read buffer from stream: %u\n", stream_data->stream_id);

        struct epoll_event ev;
        ev.events = EPOLLIN;
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL,stream_data->sock_fd, &ev);
 
        close(stream_data->sock_fd);
        socket_hashmap_remove(hashmap, stream_data->stream_id);
        
        msg_tor_relay_end_t* tor_end_msg = (msg_tor_relay_end_t*)buffer;
        tor_end_msg->relay = TOR_RELAY;
        tor_end_msg->cmd = RELAY_END;
        tor_end_msg->stream_id = stream_data->stream_id;
        if (send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
        {
            close(session->last_fd);
            close(epoll_fd);
            free_relay_session(session);

            return false;
        }

        return true;
    }

    msg_tor_relay_data_t* tor_data_msg = (msg_tor_relay_data_t*)buffer;
    tor_data_msg->relay = TOR_RELAY;
    tor_data_msg->cmd = RELAY_DATA;
    tor_data_msg->length = size;
    tor_data_msg->stream_id = stream_data->stream_id;
    if (send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
    {
        close(session->last_fd);
        close(epoll_fd);
        socket_hashmap_free(hashmap);
        free_relay_session(session);

        *response = relay_error;

        return false;
    }
    
    printf("Sent DATA len(%lu)\n", size);

    return true;
} 

static bool handle_event(relay_session_t* session, struct epoll_event* event, socket_hashmap_t* hashmap, const int epoll_fd, msg_tor_buffer_t* buffer, relay_code_t* response)
{
    if (event->data.u32 == CLIENT_STREAM_ID)
    {
        printf("Handle relay event.\n");

        if (recv_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true) == false)
        {
            close(session->last_fd);
            close(epoll_fd);
            socket_hashmap_free(hashmap);
            free_relay_session(session);

            *response = relay_error;

            return false;
        }

        return process_client_msg(session, hashmap, epoll_fd, buffer, response);
    }
    else
    {
        printf("Handle internet event.\n");

        socket_hashmap_entry_t* entry = socket_hashmap_find(hashmap, event->data.u32);
        
        if (entry == NULL)
        {
            close(session->last_fd);
            close(epoll_fd);
            socket_hashmap_free(hashmap);
            free_relay_session(session);

            *response = relay_error;

            return false;
        }
        
        return process_internet_event(&entry->data, session, hashmap, epoll_fd, buffer, response);
    }
}

static bool process_epoll_event(const int epoll_fd, relay_session_t* session, socket_hashmap_t* hashmap, msg_tor_buffer_t* buffer, relay_code_t* response)
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

    if (!process_client_msg(session, &hashmap, epoll_fd, buffer, &response))
    {
        return response;
    }

    while (true)
    {
        if (process_epoll_event(epoll_fd, session, &hashmap, buffer, &response) == false)
        {
            printf("Ended with response %d\n", response);
            return response;
        }
    }
}