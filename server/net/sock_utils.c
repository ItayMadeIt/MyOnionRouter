#include <sock_utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int create_and_bind(const config_metadata_t* config)
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
        return -1;
    }

    // Create socket for the bind
    int sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_fd == -1)
    {
        // socket() failed
        return -1;
    }

    if (bind(sock_fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        // bind() failed
        return -1;
    }

    return sock_fd;
}

void accept_loop(int server_fd, void (*accept_callback)(int client_fd))
{
    while (true)
    {
        int client_fd = accept(server_fd, NULL, NULL);

        // Accept had error
        if (client_fd < 0) continue;

        // Run accept callback with a new thread
        pthread_t thread_id;
        pthread_create(
            &thread_id,
            NULL, 
            (void *(*)(void *))accept_callback, 
            (void*)(intptr_t)client_fd
        );
        pthread_detach(thread_id);
    }
}
