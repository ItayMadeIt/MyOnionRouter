#include <relay_manager.h>

#include <protocol/relay_data_structs.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define RELAY_ITEMS_MIN_LENGTH 2
#define RELAY_STACK_MIN_LENGTH 1

static bool initialized = false;

static uint16_t items_length;
static relay_data_item_t* items;
static uint16_t max_item_index;

static uint16_t stack_length;
static uint16_t* removed_id_stack; 
static uint16_t max_stack_index;

static pthread_mutex_t relay_manager_thread = PTHREAD_MUTEX_INITIALIZER;

void init_relay_manager()
{
    pthread_mutex_lock(&relay_manager_thread);

    if (initialized)
    {
        pthread_mutex_unlock(&relay_manager_thread);
        return;
    }

    initialized = true;

    items_length = RELAY_ITEMS_MIN_LENGTH;
    items = calloc(items_length, sizeof(relay_data_item_t));
    max_item_index = 0;

    for (uint16_t i = 0; i < items_length; i++)
    {
        items[i].exists = false;
    }

    stack_length = RELAY_STACK_MIN_LENGTH;
    removed_id_stack = calloc(stack_length, sizeof(uint16_t));
    max_stack_index = 0;

    pthread_mutex_unlock(&relay_manager_thread);
}

void free_relay_manager() 
{
    pthread_mutex_lock(&relay_manager_thread);

    if (initialized == false)
    {
        pthread_mutex_unlock(&relay_manager_thread);
        return;
    }

    free(items);
    items = NULL;

    free(removed_id_stack);
    removed_id_stack = NULL;

    items_length = 0;
    stack_length = 0;

    max_item_index = 0;
    max_stack_index = 0;

    initialized = false;

    pthread_mutex_unlock(&relay_manager_thread);
}

relay_data_t* get_relay(uint16_t id)
{
    pthread_mutex_lock(&relay_manager_thread);

    relay_data_t* result = NULL;

    if (id < items_length && items[id].exists)
    {
        result = items[id].data;
    }

    pthread_mutex_unlock(&relay_manager_thread);

    return result;
}

relay_data_t* gen_relay()
{
    pthread_mutex_lock(&relay_manager_thread);

    if (max_stack_index == 0)
    {   
        // If out of space, multiply length by 2
        if (items_length <= max_item_index)
        {
            uint16_t old_length = items_length;
            items_length *= 2;

            relay_data_item_t* new_items = 
                realloc(items, items_length * sizeof(relay_data_item_t));
        
            assert(new_items != NULL);
    
            memset(new_items + old_length, 0, (items_length - old_length) * sizeof(relay_data_item_t));

            items = new_items;
        }

        // Assigns id to the index
        memset(&items[max_item_index], 0, sizeof(items[max_item_index]));

        items[max_item_index].data = calloc(1, sizeof(relay_data_t));
        items[max_item_index].data->relay_id = max_item_index;
        
        items[max_item_index].exists = true;

        relay_data_t* relay_data = items[max_item_index++].data;

        pthread_mutex_unlock(&relay_manager_thread);

        return relay_data;
    }

    max_stack_index--;
    uint16_t removed_id = removed_id_stack[max_stack_index];

    // Assigns id to the index
    memset(&items[removed_id], 0, sizeof(items[removed_id]));

    items[removed_id].data = calloc(1, sizeof(relay_data_t));
    items[removed_id].data->relay_id = removed_id;
    items[removed_id].exists = true;

    relay_data_t* relay_data = items[removed_id].data;
    
    pthread_mutex_unlock(&relay_manager_thread);
    
    return relay_data;
}

bool remove_relay(uint16_t id)
{
    pthread_mutex_lock(&relay_manager_thread);

    if (items[id].exists == false || id >= items_length)
    {
        pthread_mutex_unlock(&relay_manager_thread);

        return false;
    }

    items[id].exists = false;

    if (max_stack_index >= stack_length)
    {
        stack_length *= 2;

        uint16_t* new_removed_id_stack = 
            (uint16_t*)realloc(removed_id_stack, stack_length * sizeof(uint16_t));
    
        assert(new_removed_id_stack != NULL);

        removed_id_stack = new_removed_id_stack;
    }

    removed_id_stack[max_stack_index++] = id;

    free(items[id].data);
    items[id].data = NULL;

    pthread_mutex_unlock(&relay_manager_thread);

    return true;
}

uint16_t get_relay_batch(relay_descriptor_t* out, uint16_t* start, uint16_t max) 
{
    pthread_mutex_lock(&relay_manager_thread);

    uint16_t found = 0;
    uint16_t i = *start % items_length;
    uint16_t count = 0;


    while (found < max && count < items_length) 
    {
        if (items[i].exists) 
        {
            memcpy(&out[found++], &items[i].data->descriptor, sizeof(relay_descriptor_t));
        }

        i = (i + 1) % items_length;
        count++;
    }

    *start = i;

    pthread_mutex_unlock(&relay_manager_thread);

    return found;
}
