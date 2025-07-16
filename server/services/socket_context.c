#include <socket_context.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <threads.h>
#include <time.h>

static bool initialized = false;
static uint64_t socks_used;
static key_sock_map_t key_list;

static pthread_mutex_t key_list_mutex = PTHREAD_MUTEX_INITIALIZER;

#define KEY_LIST_START_SIZE 4

static inline bool is_valid_socket(int sock_fd) 
{
    return (uint64_t)sock_fd < key_list.length && key_list.arr[sock_fd].exists;
}


static void resize_key_list(int new_sock_fd)
{
    uint64_t last_length = key_list.length;

    while (key_list.length <= (uint64_t)new_sock_fd)
    {
        key_list.length *= 2;
    }

    key_sock_t* new_arr = realloc(key_list.arr, key_list.length * sizeof(key_sock_t));

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


void init_socket_context()
{
    pthread_mutex_lock(&key_list_mutex);

    if (initialized)
    {
        pthread_mutex_unlock(&key_list_mutex);
        return;
    }

    initialized = true;

    key_list.length = KEY_LIST_START_SIZE;
    key_list.arr = malloc(sizeof(key_sock_t) * key_list.length);

    for (uint64_t i = 0; i < key_list.length; i++)
    {
        key_list.arr[i].exists = false;
    }

    socks_used = 0;

    pthread_mutex_unlock(&key_list_mutex);
}

bool add_socket(int sock_fd, key_data_t* key, void* data)
{
    pthread_mutex_lock(&key_list_mutex);

    if ((uint64_t)sock_fd >= key_list.length)
    {    
        resize_key_list(sock_fd);
    }

    if (key_list.arr[sock_fd].exists)
    {
        pthread_mutex_unlock(&key_list_mutex);
        return false;
    }
    
    key_list.arr[sock_fd].exists = true;    

    key_list.arr[sock_fd].data = data;

    memcpy(&key_list.arr[sock_fd].key_data, key, sizeof(key_data_t));   


    socks_used++;

    pthread_mutex_unlock(&key_list_mutex);
    return true;
}

bool assign_sock_data(int sock_fd, void* data) 
{
    pthread_mutex_lock(&key_list_mutex);

    if (is_valid_socket(sock_fd) == false)
    {
        pthread_mutex_unlock(&key_list_mutex);
        return false;
    }

    key_list.arr[sock_fd].data = data;

    pthread_mutex_unlock(&key_list_mutex);
    return true;
}

void* get_sock_data(int sock_fd) 
{
    pthread_mutex_lock(&key_list_mutex);

    if (is_valid_socket(sock_fd) == false) 
    {
        pthread_mutex_unlock(&key_list_mutex);
        return NULL;
    }

    void* data = key_list.arr[sock_fd].data;

    pthread_mutex_unlock(&key_list_mutex);
    return data;
}


void free_sock_data(int sock_fd, void(*free_data)(void*)) 
{
    if (free_data == NULL)
    {
        return;
    }

    pthread_mutex_lock(&key_list_mutex);

    if (is_valid_socket(sock_fd) == false) 
    {
        pthread_mutex_unlock(&key_list_mutex);
        return;
    }

    free_data(key_list.arr[sock_fd].data);
    key_list.arr[sock_fd].data = NULL;

    pthread_mutex_unlock(&key_list_mutex);
}



key_data_t* get_sock_key(int sock_fd)
{
    pthread_mutex_lock(&key_list_mutex);

    if (is_valid_socket(sock_fd) == false)
    {
        pthread_mutex_unlock(&key_list_mutex);
        return NULL;
    }

    key_data_t* key = &key_list.arr[sock_fd].key_data;
    pthread_mutex_unlock(&key_list_mutex);
    return key;
}

void free_socket(int sock_fd, void(*free_data)(void*))
{
    pthread_mutex_lock(&key_list_mutex);

    if (is_valid_socket(sock_fd) == false)
    {
        pthread_mutex_unlock(&key_list_mutex);
        return;
    }

    key_list.arr[sock_fd].exists = false;

    socks_used--;
    
    free_key(&key_list.arr[sock_fd].key_data);
    
    if (free_data != NULL) 
    {
        free_data(key_list.arr[sock_fd].data);
    }
    
    pthread_mutex_unlock(&key_list_mutex);
}

// Assumes all socket data has already been freed via free_socket_data()
bool free_socket_context()
{
    pthread_mutex_lock(&key_list_mutex);

    if (socks_used != 0) 
    {
        pthread_mutex_unlock(&key_list_mutex);
        return false;
    }

    free(key_list.arr);
    key_list.arr = NULL;

    key_list.length = 0;

    pthread_mutex_unlock(&key_list_mutex);
    
    return true;
}