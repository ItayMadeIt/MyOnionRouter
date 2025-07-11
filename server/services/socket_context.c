#include <socket_context.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

key_sock_map_t key_list;
uint64_t socks_used;

#define KEY_LIST_START_SIZE 4

void init_socket_context()
{
    key_list.length = KEY_LIST_START_SIZE;
    key_list.arr = (key_sock_t*)malloc(sizeof(key_sock_t) * key_list.length);

    for (uint64_t i = 0; i < key_list.length; i++)
    {
        key_list.arr[i].exists = false;
    }

    socks_used = 0;
}

static void resize_key_list(int new_sock_fd)
{
    uint64_t last_length = key_list.length;

    while (key_list.length <= (uint64_t)new_sock_fd)
    {
        key_list.length *= 2;
    }

    key_sock_t* new_arr = (key_sock_t*)realloc(key_list.arr, key_list.length * sizeof(key_sock_t));

    if (new_arr == NULL)
    {
        fprintf(stderr, "Reallocate key_list failed, new length: %lu\n", key_list.length);
        abort();
    }

    key_list.arr = new_arr;

    for (uint64_t i = last_length; i < key_list.length; i++)
    {
        key_list.arr[i].exists = false;
    }
}

bool add_socket(int sock_fd, key_data_t* key)
{
    if ((uint64_t)sock_fd < key_list.length && key_list.arr[sock_fd].exists)
    {
        return false;
    }

    resize_key_list(sock_fd);

    key_list.arr[sock_fd].exists = true;    

    memcpy(&key_list.arr[sock_fd].key_data, key, sizeof(key_data_t));   

    socks_used++;

    return true;
}

key_data_t* get_key(int sock_fd)
{
    if ((uint64_t)sock_fd >= key_list.length)
    {
        return NULL;
    }

    if (key_list.arr[sock_fd].exists == false)
    {
        return NULL;
    }

    return &key_list.arr[sock_fd].key_data;
}

void free_socket_data(int sock_fd)
{

    if ((uint64_t)sock_fd >= key_list.length)
    {
        return;
    }

    if (key_list.arr[sock_fd].exists == false)
    {
        return;
    }

    key_list.arr[sock_fd].exists = false;

    socks_used--;
    
    free_key(&key_list.arr[sock_fd].key_data);
}

void free_socket_context()
{
    free(key_list.arr);
    key_list.arr = NULL;

    key_list.length = 0;
}