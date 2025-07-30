#include "session.h"

#include "client.h"

#include <handle_tls.h>

#include <netdb.h>
#include <unistd.h>
#include <utils/sock_utils.h>
#include <server_query.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

client_vars_t client_vars;

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


client_code_t run_client()
{
    init_encryption();

    circuit_relay_list_t relay_list;
    gather_relay_map(&relay_list, client_vars.config->relays);

    struct sockaddr_storage storage;
    socklen_t length;
    sockaddr_to_unix(&storage, &length, &relay_list.relays[0].sock_addr);
    int sock_fd = connect_server_by_sockaddr(&storage, length);

    // init session
    client_session_t session;
    session.relays = &relay_list;
    init_session(&session, sock_fd, &relay_list);

    // tls handshake
    if (handle_tls_sender(session.sock_fd, &session.tls_key) == false)
    {
        session.sock_fd = -1;
        free_session(&session);
        return client_tls_error;
    }

    client_code_t result = process_client_session(&session);
    if (result != client_success)
    {
        free_encryption();

        return client_error;
    }

    free_session(&session);

    free_encryption();

    return client_success;
}