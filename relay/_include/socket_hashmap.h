#ifndef __SOCKET_HASHMAP_H__
#define __SOCKET_HASHMAP_H__

#include <stdint.h>
#include <stdbool.h>

#include "stream_data_manager.h"

// Hashmap amount, can't be higher than 2048 (meaning 2048 sockets)
// 2^n for fast index%length speed up
#define HASHMAP_AMOUNT 2048

typedef enum hashmap_entry_state
{
    HASHMAP_STATE_EMPTY,
    HASHMAP_STATE_FILLED,
    HASHMAP_STATE_DELETED,
} hashmap_entry_state_t;

typedef struct socket_hashmap_entry
{
    stream_buffer_t buffer;
    int socket_fd;

    uint16_t stream_id;
    uint8_t entry_type;
} socket_hashmap_entry_t;

typedef struct socket_hashmap
{
    socket_hashmap_entry_t entries[HASHMAP_AMOUNT];
} socket_hashmap_t;

void socket_hashmap_init(socket_hashmap_t* map);
socket_hashmap_entry_t* socket_hashmap_insert(socket_hashmap_t* map, uint16_t stream_id, int socket_fd);
socket_hashmap_entry_t* socket_hashmap_find(socket_hashmap_t* map, uint16_t stream_id);
bool socket_hashmap_remove(socket_hashmap_t* map, uint16_t stream_id);

#endif // __SOCKET_HASHMAP_H__