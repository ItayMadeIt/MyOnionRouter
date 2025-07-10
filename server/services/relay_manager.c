#include <relay_manager.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RELAY_ITEMS_MIN_LENGTH 2
#define RELAY_STACK_MIN_LENGTH 1

static uint32_t items_length;
static relay_data_item_t* items;
static uint32_t max_item_index;

static uint32_t stack_length;
static uint32_t* removed_id_stack; 
static uint32_t max_stack_index;

void setup_relay_manager()
{
    items_length = RELAY_ITEMS_MIN_LENGTH;
    items = (relay_data_item_t*)calloc(items_length, sizeof(relay_data_item_t));
    max_item_index = 0;

    for (uint32_t i = 0; i < items_length; i++)
    {
        items[i].exists = false;
    }

    stack_length = RELAY_STACK_MIN_LENGTH;
    removed_id_stack = (uint32_t*)calloc(stack_length, sizeof(uint32_t));
    max_stack_index = 0;
}

relay_data_t* get_relay(uint32_t id)
{
    if (items[id].exists)
    {
        return &items[id].data;
    }

    return NULL;
}

relay_data_t* gen_relay()
{
    if (max_stack_index == 0)
    {   
        // If out of space, multiply length by 2
        if (items_length <= max_item_index)
        {
            items_length *= 2;

            relay_data_item_t* new_items = 
                (relay_data_item_t*)realloc(items, items_length * sizeof(relay_data_item_t));
        
            assert(new_items != NULL);

            items = new_items;
        }

        // Assigns id to the index
        memset(&items[max_item_index], 0, sizeof(items[max_item_index]));

        items[max_item_index].data.descriptor.relay_id = max_item_index;
        items[max_item_index].exists = true;

        return &items[max_item_index++].data;
    }

    max_stack_index--;
    uint32_t removed_id = removed_id_stack[max_stack_index];

    // Assigns id to the index
    memset(&items[removed_id], 0, sizeof(items[removed_id]));

    items[removed_id].data.descriptor.relay_id = removed_id;
    items[removed_id].exists = true;

    return &items[removed_id].data;
}

bool remove_relay(uint32_t id)
{
    if (items[id].exists == false || id >= items_length)
    {
        return false;
    }

    items[id].exists = false;

    if (max_stack_index >= stack_length)
    {
        stack_length *= 2;

        uint32_t* new_removed_id_stack = 
            (uint32_t*)realloc(removed_id_stack, stack_length * sizeof(uint32_t));
    
        assert(removed_id_stack != NULL);

        removed_id_stack = new_removed_id_stack;
    }

    removed_id_stack[max_stack_index++] = id;

    return true;
}