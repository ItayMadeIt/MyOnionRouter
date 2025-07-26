#include "stream_data_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUFFER_CAPACITY 2
#define INITIAL_STACK_CAPACITY 2

static void stream_data_manager_grow_buffers(stream_data_manager_t *manager) 
{
    uint32_t old_capacity = manager->buffer_capacity;
    manager->buffer_capacity *= 2;

    manager->buffers = realloc(manager->buffers, manager->buffer_capacity * sizeof(stream_data_buffer_t));
    if (!manager->buffers) 
    {
        fprintf(stderr, "Failed to grow buffers\n");
        exit(EXIT_FAILURE);
    }
    memset(&manager->buffers[old_capacity], 0, (manager->buffer_capacity - old_capacity) * sizeof(stream_data_buffer_t));
}

static void stream_data_manager_grow_stack(stream_data_manager_t *manager) 
{
    manager->free_capacity *= 2;
    manager->free_stack = realloc(manager->free_stack, manager->free_capacity * sizeof(uint32_t));
    if (!manager->free_stack) 
    {
        fprintf(stderr, "Failed to grow free stack\n");
        exit(EXIT_FAILURE);
    }
}


void stream_data_manager_init(stream_data_manager_t* manager)
{
    manager->buffer_capacity = INITIAL_BUFFER_CAPACITY;
    manager->next_index = 0;
    manager->buffers = calloc(manager->buffer_capacity, sizeof(stream_data_buffer_t));

    manager->free_capacity = INITIAL_STACK_CAPACITY;
    manager->free_top = 0;
    manager->free_stack = malloc(manager->free_capacity * sizeof(uint32_t));
}

uint32_t stream_data_manager_add(stream_data_manager_t *manager) 
{
    uint32_t index;

    if (manager->free_top > 0) 
    {
        index = manager->free_stack[--manager->free_top];
    } 
    else 
    {
        if (manager->next_index == manager->buffer_capacity)
        {
            stream_data_manager_grow_buffers(manager);
        }

        index = manager->next_index++;
    }

    manager->buffers[index].exists = 1;
    return (uint32_t)index;
}

void stream_data_manager_remove(stream_data_manager_t *manager, uint32_t index) 
{
    if (index >= manager->buffer_capacity) 
    {
        return;
    }

    if (manager->buffers[index].exists == 0)
    { 
        return;
    }

    manager->buffers[index].exists = 0;
    if (manager->free_top == manager->free_capacity)
    {
        stream_data_manager_grow_stack(manager);
    }
    manager->free_stack[manager->free_top++] = (uint32_t)index;
}

void* stream_data_manager_get_data(stream_data_manager_t *manager, uint32_t index) 
{
    if (index >= manager->buffer_capacity) 
    {
        return NULL;
    }

    if (manager->buffers[index].exists == 0)
    {
        return NULL;
    }
    
    return manager->buffers[index].data;
}

void stream_data_manager_free(stream_data_manager_t *manager)
{
    if (manager->buffers)
    {
        free(manager->buffers);
    }
    
    if (manager->free_stack)
    {    
        free(manager->free_stack);
    }

    manager->buffers = NULL;
    manager->free_stack = NULL;
    manager->buffer_capacity = 0;
    manager->next_index = 0;
    manager->free_capacity = 0;
    manager->free_top = 0;
}
