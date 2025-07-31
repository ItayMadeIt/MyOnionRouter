#include "stream_hashmap.h"
#include "stream_buffers.h"
#include <stddef.h>
#include <unistd.h>

void socket_hashmap_init(stream_hashmap_t *map)
{
    for (uint16_t i = 0; i < HASHMAP_AMOUNT; i++)
    {
        map->entries[i].entry_type = HASHMAP_STATE_EMPTY;
    }
}

void socket_hashmap_free(stream_hashmap_t *map)
{
    for (uint16_t i = 0; i < HASHMAP_AMOUNT; i++)
    {
        if (map->entries[i].entry_type == HASHMAP_STATE_FILLED)
        {
            free_stream_buffer(&map->entries[i].data.recv_buffer);
        }
    }
}

stream_hashmap_entry_t* socket_hashmap_insert(stream_hashmap_t *map, uint16_t stream_id)
{
    uint16_t start_index = stream_id % HASHMAP_AMOUNT;
    uint16_t index = start_index; 

    while (map->entries[index].entry_type == HASHMAP_STATE_FILLED)
    {
        if (map->entries[index].data.stream_id == stream_id)
        {
            return NULL;
        }

        index = (index + 1) % HASHMAP_AMOUNT;

        if (index == start_index)
        {
            return NULL;
        }
    }

    map->entries[index].entry_type = HASHMAP_STATE_FILLED;
 
    map->entries[index].data.stream_id = stream_id;

    init_stream_buffer(&map->entries[index].data.recv_buffer);

    return &map->entries[index];
}

stream_hashmap_entry_t *socket_hashmap_find(stream_hashmap_t *map, uint16_t stream_id)
{
    uint16_t start_index = stream_id % HASHMAP_AMOUNT;
    uint16_t index = start_index; 

    while (map->entries[index].entry_type != HASHMAP_STATE_EMPTY)
    {
        if (map->entries[index].entry_type == HASHMAP_STATE_FILLED && 
               map->entries[index].data.stream_id == stream_id)
        {
            return &map->entries[index];
        }

        index = (index + 1) % HASHMAP_AMOUNT;

        if (index == start_index)
        {
            return NULL;
        }
    }

    return NULL;
}

bool socket_hashmap_remove(stream_hashmap_t *map, uint16_t stream_id)
{
    uint16_t start_index = stream_id % HASHMAP_AMOUNT;
    uint16_t index = start_index; 

    while (map->entries[index].entry_type != HASHMAP_STATE_EMPTY)
    {
        if (map->entries[index].entry_type == HASHMAP_STATE_FILLED && 
               map->entries[index].data.stream_id == stream_id)
        {
            map->entries[index].entry_type = HASHMAP_STATE_DELETED;
            free_stream_buffer(&map->entries[index].data.recv_buffer);
            return true;
        }

        index = (index + 1) % HASHMAP_AMOUNT;

        if (index == start_index)
        {
            return false;
        }
    }

    return false;
}

uint32_t gen_stream_id(stream_hashmap_t* hashmap) 
{ 
    static uint32_t stream_id = 0;
    
    while (hashmap->entries[stream_id % HASHMAP_AMOUNT].entry_type == HASHMAP_STATE_FILLED)
    {
        stream_id++;
    }

    return stream_id;
}
