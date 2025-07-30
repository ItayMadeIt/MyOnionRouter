#include "sock_utils.h"
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

int connect_server_by_sockaddr(struct sockaddr_storage* addr, socklen_t addr_len)
{
    int sock = socket(addr->ss_family, SOCK_STREAM, 0);
    if (sock == -1) 
    {
        fprintf(stderr, "Failed to create socket\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)addr, addr_len) == -1) 
    {
        fprintf(stderr, "Failed to connect\n");
        close(sock);
        return -1;
    }

    return sock;  
}
