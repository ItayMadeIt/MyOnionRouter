#ifndef __STREAM_DATA_MANAGER_H__
#define __STREAM_DATA_MANAGER_H__

#include <stdint.h>

#define STREAM_BUFFER_SIZE 4096

typedef struct circular_buffer
{
    void* buffer;
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t capacity;
} circular_buffer_t;

void init_stream_buffer (circular_buffer_t* stream_buffer);
void stream_push_data   (circular_buffer_t* buffer, const uint8_t* data, const uint32_t size);
uint32_t stream_pop_data(circular_buffer_t* buffer, uint8_t* data, const uint32_t size);
void free_stream_buffer (circular_buffer_t* stream_buffer);

#endif // __STREAM_DATA_MANAGER_H__
