#include "client.h"
#include <stdio.h>

client_code_t run_client(const client_config_metadata_t* client_config)
{
    printf("relays: %d\n", client_config->relays);
    printf("server: %s:%s\n", client_config->server_cfg->server, client_config->server_cfg->port);

    return client_success;
}