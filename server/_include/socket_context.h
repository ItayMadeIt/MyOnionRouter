#ifndef __SOCKET_CONTEXT_H__
#define __SOCKET_CONTEXT_H__

#include <encryptions/encryptions.h>

typedef struct key_sock {
    bool exists;
    key_data_t key_data;
    void* data; // used for relay_descriptor for example. on client = NULL
} key_sock_t;

typedef struct key_sock_map {
    key_sock_t* arr;
    uint64_t length;
} key_sock_map_t;

void init_socket_context();

bool add_socket(int sock_fd, key_data_t* key, void* data);

key_data_t* get_sock_key(int sock_fd);

bool assign_sock_data(int sock_fd, void* data);
void* get_sock_data(int sock_fd);
void free_sock_data(int sock_fd, void(*free_data)(void*));

void free_socket(int sock_fd, void(*free_data)(void*));

bool free_socket_context();
#endif // __SOCKET_CONTEXT_H__