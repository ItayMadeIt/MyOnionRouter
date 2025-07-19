#include <stdint.h>

#define RELAY_MSG_SIZE 512

typedef struct msg_tor_buffer {
    uint8_t data[RELAY_MSG_SIZE];
} msg_tor_buffer_t;
