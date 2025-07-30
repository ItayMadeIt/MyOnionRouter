#include <fcntl.h>

void set_socket_nonblock(int sock_fd) 
{
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);
}
