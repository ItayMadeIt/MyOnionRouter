#include "sock_utils.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int connect_server(const server_config_metadata_t* config)
{
    struct addrinfo hints = {0}, *res, *ptr;
    hints.ai_family = AF_UNSPEC;  // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(config->server, config->port, &hints, &res);
    if (err != 0) 
    {
        return -1;
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        int sock_fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        
        if (sock_fd == -1)
        {
            continue;
        }

        if (connect(sock_fd, ptr->ai_addr, ptr->ai_addrlen) == -1)
        {
            close(sock_fd);
            continue;
        }

        freeaddrinfo(res);
    
        return sock_fd;
    }
    
    freeaddrinfo(res);

    return -1;
}