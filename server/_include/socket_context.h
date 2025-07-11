#include <encryptions/encryptions.h>

typedef struct key_sock {
    bool exists;
    key_data_t key_data;
} key_sock_t;

typedef struct key_sock_map {
    key_sock_t* arr;
    uint64_t length;
} key_sock_map_t;

extern key_sock_map_t key_list;

void init_socket_context();

bool add_socket(int sock_fd, key_data_t* key);
key_data_t* get_key(int sock_fd);
void free_socket_data(int sock_fd);

void free_socket_context();