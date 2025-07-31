#ifndef __SOCKET_HASHMAP_H__
#define __SOCKET_HASHMAP_H__

#include <stdint.h>
#include <stdbool.h>

#include "stream_buffers.h"

// Hashmap amount, can't be higher than 2048 (meaning 2048 sockets)
// 2^n for fast index%length speed up
#define HASHMAP_AMOUNT 2048

typedef struct stream_data {
    circular_buffer_t recv_buffer;
    uint16_t stream_id;
} stream_data_t;

typedef enum hashmap_entry_state
{
    HASHMAP_STATE_EMPTY,
    HASHMAP_STATE_FILLED,
    HASHMAP_STATE_DELETED,
} hashmap_entry_state_t;

typedef struct stream_hashmap_entry
{
    stream_data_t data;
    uint8_t entry_type;
} stream_hashmap_entry_t;

typedef struct socket_hashmap
{
    stream_hashmap_entry_t entries[HASHMAP_AMOUNT];
} stream_hashmap_t;

void socket_hashmap_init(stream_hashmap_t* map);
void socket_hashmap_free(stream_hashmap_t* map);

stream_hashmap_entry_t* socket_hashmap_insert(stream_hashmap_t* map, uint16_t stream_id);
stream_hashmap_entry_t* socket_hashmap_find(stream_hashmap_t* map, uint16_t stream_id);
bool socket_hashmap_remove(stream_hashmap_t* map, uint16_t stream_id);

uint32_t gen_stream_id(stream_hashmap_t* hashmap) ;

#endif // __SOCKET_HASHMAP_H__