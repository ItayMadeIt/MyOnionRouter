#include "sock_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int create_and_bind(const server_config_metadata_t *config, struct sockaddr_storage* bind_addr)
{
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;      
    hints.ai_socktype = SOCK_STREAM;   
    hints.ai_flags    = AI_PASSIVE; 

    // Get addr info (prepare bind to 0.0.0.0)
    if (getaddrinfo(NULL, config->port, &hints, &res) != 0) 
    {
        // getaddrinfo() failed
        printf("Failed to getaddr on %s...\n", config->port);
        return -1;
    }

    // Create socket for the bind
    int sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_fd == -1)
    {
        // socket() failed
        printf("Failed to create socket %s...\n", config->port);
        freeaddrinfo(res);
        return -1;
    }

    int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(sock_fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        // bind() failed
        printf("Failed to bind on %s...\n", config->port);
        freeaddrinfo(res);
        return -1;
    }

    if (bind_addr != NULL)
    {
        memcpy(bind_addr, res->ai_addr, res->ai_addrlen);
    }

    printf("Listening on port %s...\n", config->port);

    freeaddrinfo(res);
    return sock_fd;
}