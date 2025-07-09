#ifndef __SOCK_UTILS_H__
#define __SOCK_UTILS_H__

#include <config.h>

// Creates a socket and binds it using config
//  returns new sockfd if success, otherwise -1
int create_and_bind(const config_metadata_t* config);

// Runs an infinite synchronous loop that accepts client, 
//  each client gets it's fd called in an accept callback function. 
void accept_loop(int server_fd, void (*accept_callback)(int client_fd));

#endif // __SOCK_UTILS_H__