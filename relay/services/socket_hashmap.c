#include "socket_hashmap.h"
#include <stddef.h>

void socket_hashmap_init(socket_hashmap_t *map)
{
    for (uint16_t i = 0; i < HASHMAP_AMOUNT; i++)
    {
        map->entries[i].entry_type = HASHMAP_STATE_EMPTY;
    }
}

bool socket_hashmap_insert(socket_hashmap_t *map, uint16_t stream_id, int socket_fd, uint32_t data_index)
{
    uint16_t start_index = stream_id % HASHMAP_AMOUNT;
    uint16_t index = start_index; 

    while (map->entries[index].entry_type == HASHMAP_STATE_FILLED)
    {
        if (map->entries[index].stream_id == stream_id)
        {
            return false;
        }

        index = (index + 1) % HASHMAP_AMOUNT;

        if (index == start_index)
        {
            return false;
        }
    }

    map->entries[index].entry_type = HASHMAP_STATE_FILLED;
 
    map->entries[index].stream_id = stream_id;
    map->entries[index].socket_fd = socket_fd;
    map->entries[index].data_index = data_index;

    return true;
}

socket_hashmap_entry_t *socket_hashmap_find(socket_hashmap_t *map, uint16_t stream_id)
{
    uint16_t start_index = stream_id % HASHMAP_AMOUNT;
    uint16_t index = start_index; 

    while (map->entries[index].entry_type != HASHMAP_STATE_EMPTY)
    {
        if (map->entries[index].entry_type == HASHMAP_STATE_FILLED && 
               map->entries[index].stream_id == stream_id)
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

bool socket_hashmap_remove(socket_hashmap_t *map, uint16_t stream_id)
{
    uint16_t start_index = stream_id % HASHMAP_AMOUNT;
    uint16_t index = start_index; 

    while (map->entries[index].entry_type != HASHMAP_STATE_EMPTY)
    {
        if (map->entries[index].entry_type == HASHMAP_STATE_FILLED && 
               map->entries[index].stream_id == stream_id)
        {
            map->entries[index].entry_type = HASHMAP_STATE_DELETED;
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
