#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include <stdbool.h>

typedef struct server_config_metadata
{
    char* port;
    char* server;
} server_config_metadata_t;

bool fetch_server_config(const char* filepath, server_config_metadata_t* metadata);
void free_server_config(server_config_metadata_t* metadata);

#endif // __SERVER_CONFIG_H__