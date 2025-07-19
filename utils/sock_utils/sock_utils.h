#ifndef __SOCK_UTILS_H__
#define __SOCK_UTILS_H__

#include <stdint.h>
#include <stdbool.h>

#include <server_cfg/server_config.h>
#include <sys/socket.h>

typedef struct user_descriptor {
    int fd;
    struct sockaddr_storage addr;
    socklen_t addr_len;
} user_descriptor_t;

// Creates a socket and binds it using config
//  returns new sockfd if success, otherwise -1
int create_and_bind(const server_config_metadata_t* config);

// Runs an infinite synchronous loop that accepts client, 
//  each client gets it's fd called in an accept callback function. 
void accept_loop(int server_fd, void (*accept_callback)(user_descriptor_t* user));

// Connects to a server based on server_config_metadata_t
//  returns the new sock_fd
int connect_server(const server_config_metadata_t* config);

// Connects to a server based on sockaddr
//  returns the new sock_fd
int connect_server_by_sockaddr(struct sockaddr_storage* addr, socklen_t addr_len);

#endif // __SOCK_UTILS_H__