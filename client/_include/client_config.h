#ifndef __CLIENT_CONFIG_H__
#define __CLIENT_CONFIG_H__

#include <utils/server_config.h>

#include <stdint.h>

typedef struct client_config_metadata
{
    server_config_metadata_t* server_cfg;

    uint8_t relays; 

} client_config_metadata_t;

bool parse_args(const int argc, const char** argv, client_config_metadata_t** client_config);

#endif // __CLIENT_CONFIG_H__