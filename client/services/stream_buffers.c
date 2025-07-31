#include "stream_buffers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Must be 2^n
#define INITIAL_BUFFER_CAPACITY 64

static void init_buffer(circular_buffer_t* buffer)
{
    buffer->buffer = (uint8_t*)malloc(INITIAL_BUFFER_CAPACITY);
    if (buffer->buffer == NULL)
    {
        abort();
    }
    
    buffer->size = 0;
    buffer->tail = 0;
    buffer->head = 0;
    buffer->capacity = INITIAL_BUFFER_CAPACITY;
}

static void free_buffer(circular_buffer_t* buffer)
{
    free(buffer->buffer);
}

static void grow_buffer(circular_buffer_t* buffer)
{
    uint32_t old_capacity = buffer->capacity;
    buffer->capacity *= 2;

    buffer->buffer = realloc(buffer->buffer, buffer->capacity);
    if (buffer->buffer == NULL)
    {
        fprintf(stderr, "Failed to reallocated new capacity %d\n", buffer->capacity);
        abort();
    }

    if (buffer->head < buffer->tail)
    {
        uint32_t tail_to_end_length = old_capacity - buffer->tail;
        memcpy(buffer->buffer + buffer->capacity - tail_to_end_length, buffer->buffer + buffer->tail, tail_to_end_length);

        buffer->tail += old_capacity;
    }
}

void init_stream_buffer(circular_buffer_t* buffer)
{
    init_buffer(buffer);
}

void stream_push_data(circular_buffer_t *buffer, const uint8_t *data, const uint32_t size)
{
    while (buffer->size + size > buffer->capacity)
    {
        grow_buffer(buffer);
    }

    if (buffer->head + size > buffer->capacity)
    {   
        uint32_t circular_offset = buffer->capacity - buffer->head;
        uint32_t circular_data_size = size - circular_offset;

        memcpy(buffer->buffer + buffer->head, data, circular_offset);
        memcpy(buffer->buffer, data + circular_offset, circular_data_size);

        buffer->head = circular_data_size;
    }
    else
    {
        memcpy(buffer->buffer + buffer->head, data, size);

        buffer->head += size;
    }

    buffer->size += size;
}

uint32_t stream_pop_data(circular_buffer_t *buffer, uint8_t *data, const uint32_t size)
{
    if (size == 0 || buffer->size == 0)
    {
        return 0;
    }

    uint32_t read_size = size;
    if (size > buffer->size)
    {
        read_size = buffer->size;
    }

    if (buffer->tail + read_size > buffer->capacity)
    {
        uint32_t circular_size = buffer->capacity - buffer->tail;

        memcpy(data, buffer->buffer + buffer->tail, circular_size);
        memcpy(data + circular_size, buffer->buffer, read_size - circular_size);
    }
    else
    {
        memcpy(data, buffer->buffer + buffer->tail, read_size);
    }

    buffer->tail = (buffer->tail + read_size) & (buffer->capacity - 1);
    buffer->size -= read_size;

    return read_size;
}

void free_stream_buffer(circular_buffer_t* buffer)
{
    free_buffer(buffer);
}
