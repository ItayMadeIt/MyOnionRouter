#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct config_metadata
{
    char* port;
    char* server;
} config_metadata_t;

bool fetch_config(const char* filepath, config_metadata_t* metadata);
#endif // __CONFIG_H__