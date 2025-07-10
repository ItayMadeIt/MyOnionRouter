#include "server_config.h"

#include <stdlib.h>

void free_server_config(server_config_metadata_t* metadata)
{
    free(metadata->server);
    metadata->server = NULL;
    
    free(metadata->port);
    metadata->port = NULL;
}
