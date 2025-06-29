#include <stdint.h>
#include <stdbool.h>

typedef struct config_metadata
{
    uint16_t port;
    char* server;
    uint8_t relays;
} config_metadata_t;
