#include <client.h>

#include <netdb.h>
#include <unistd.h>
#include <utils/sock_utils.h>
#include <server_query.h>
#include <stdio.h>
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

    gather_relay_map(&client_vars.relays);

    struct sockaddr_storage storage;
    socklen_t length;
    sockaddr_to_unix(&storage, &length, &client_vars.relays.relays[0].sock_addr);
    
    int sock_fd = connect_server_by_sockaddr(&storage, length);

    // Now do tor things 

    printf("%d\n\n", sock_fd);
    close(sock_fd);

    printf("Relays: %d\n", client_vars.relays.relay_amount);

    free_encryption();

    return client_success;
}