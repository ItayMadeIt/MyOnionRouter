#include "sock_utils.h"

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

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
