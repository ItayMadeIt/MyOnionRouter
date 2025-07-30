#include <relay_config.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <utils/string_utils.h>

bool parse_args(const int argc, const char** argv, relay_config_metadata_t** relay_config)
{
    *relay_config = (relay_config_metadata_t*)calloc(1, sizeof(relay_config_metadata_t));
    
    if (!*relay_config) return false;

    bool port_found = false;
    bool server_cfg_found = false;

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) && i + 1 < argc
            && (*relay_config)->relay_server_cfg.port == NULL && (*relay_config)->relay_server_cfg.server == NULL)
        {
            i++;

            (*relay_config)->relay_server_cfg.port = (char*)clone_str(argv[i]);
            (*relay_config)->relay_server_cfg.server = (char*)clone_str("0.0.0.0");

            port_found = true;
        }
        else if ((*relay_config)->dir_server_cfg.server == NULL && (*relay_config)->dir_server_cfg.port == NULL)
        {
            fetch_server_config(argv[i], &(*relay_config)->dir_server_cfg);

            server_cfg_found = (*relay_config)->dir_server_cfg.server != NULL && 
                            (*relay_config)->dir_server_cfg.port != NULL;
        }
    }

    return server_cfg_found && port_found;
}

void free_relay_config(relay_config_metadata_t* relay_config)
{
    free_server_config(&relay_config->relay_server_cfg);
    free_server_config(&relay_config->dir_server_cfg);
}