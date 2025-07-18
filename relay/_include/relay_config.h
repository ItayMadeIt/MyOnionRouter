#ifndef __CONFIGS_H__
#define __CONFIGS_H__

#include <stdint.h>
#include <stdbool.h>

#include <utils/server_config.h>

typedef struct relay_config_metadata
{
    server_config_metadata_t dir_server_cfg;

    server_config_metadata_t relay_server_cfg;

} relay_config_metadata_t;

bool parse_args(const int argc, const char** argv, relay_config_metadata_t** relay_config);
void free_relay_config(relay_config_metadata_t* relay_config);

#endif // __CONFIGS_H__