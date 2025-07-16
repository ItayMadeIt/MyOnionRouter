#include "sock_utils.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

void accept_loop(int server_fd, void (*connection_handler)(user_descriptor_t* user))
{
    while (true)
    {
        struct sockaddr_storage client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);

        // Accept had error
        if (client_fd < 0) continue;

        user_descriptor_t* user_desc = malloc(sizeof(user_descriptor_t)); 
        user_desc->fd = client_fd;
        user_desc->addr = client_addr;
        user_desc->addr_len = addr_len;

        // Run accept callback with a new thread
        pthread_t thread_id;
        if (pthread_create(
            &thread_id,
            NULL, 
            (void *(*)(void *))connection_handler, 
            (void*)user_desc) != 0)
        {
            close(client_fd);
            free(user_desc);
            continue;
        }
        pthread_detach(thread_id);
    }
}
