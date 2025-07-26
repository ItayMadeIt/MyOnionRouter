#ifndef __STREAM_DATA_MANAGER_H__
#define __STREAM_DATA_MANAGER_H__

#include <stdint.h>

#define STREAM_BUFFER_SIZE 4096
#define STREAM_METADATA_SIZE 1
#define STREAM_DATA_SIZE     (STREAM_BUFFER_SIZE - STREAM_METADATA_SIZE)


typedef struct stream_data_buffer{
    union {
        uint8_t buffer[STREAM_BUFFER_SIZE];
        struct __attribute__((packed)){
            uint8_t exists;
            uint8_t data[STREAM_DATA_SIZE];
        };
    };
} stream_data_buffer_t;

typedef struct stream_data_manager {
    stream_data_buffer_t *buffers;   // Array of stream buffers
    uint32_t buffer_capacity;        // Allocated buffer slots
    uint32_t next_index;             // Number of active buffers

    uint32_t *free_stack;            // Stack of freed indices
    uint32_t free_capacity;          // Allocated stack size
    uint32_t free_top;               // Stack top
} stream_data_manager_t;


void stream_data_manager_init(stream_data_manager_t* manager);
uint32_t stream_data_manager_add(stream_data_manager_t *manager);
void stream_data_manager_remove(stream_data_manager_t *manager, uint32_t index);
void* stream_data_manager_get_data(stream_data_manager_t *manager, uint32_t index);
void stream_data_manager_free(stream_data_manager_t* manager);

#endif // __STREAM_DATA_MANAGER_H__