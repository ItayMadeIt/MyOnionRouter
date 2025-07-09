#ifndef __CONFIGS_H__
#define __CONFIGS_H__


#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct server_config_metadata
{
    char* port;
    char* server;
} server_config_metadata_t;


typedef struct relay_config_metadata
{
    server_config_metadata_t* server_cfg;

    char* relay_port;

} relay_config_metadata_t;

bool parse_args(const int argc, const char** argv, relay_config_metadata_t** relay_config);
void free_relay_config(relay_config_metadata_t* relay_config);

bool fetch_server_config(const char* filepath, server_config_metadata_t** server_config);
void free_server_config(server_config_metadata_t* server_config);

#endif // __CONFIGS_H__