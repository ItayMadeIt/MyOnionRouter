#include <configs.h>
#include <utils/string_utils.h>
#include <stdio.h>
#include <stdlib.h>

bool parse_args(const int argc, const char** argv, relay_config_metadata_t** relay_config)
{
    *relay_config = (relay_config_metadata_t*)calloc(1, sizeof(relay_config_metadata_t));
    
    if (!*relay_config) return false;

    bool port_found = false;
    bool server_cfg_found = false;

    for (int i = 0; i < argc; i++)
    {
        if ((strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) && i + 1 < argc)
        {
            i++;

            (*relay_config)->relay_port = clone_str(argv[i]);

            port_found = true;
        }
        else if ((*relay_config)->server_cfg == NULL)
        {
            fetch_server_config(argv[i], &(*relay_config)->server_cfg);

            server_cfg_found = (*relay_config)->server_cfg != NULL;
        }
    }

    return server_cfg_found && port_found;
}

void free_relay_config(relay_config_metadata_t* relay_config)
{
    free(relay_config->relay_port);
    relay_config->relay_port = NULL;
    
    free_server_config(relay_config->server_cfg);
    free(relay_config->server_cfg);
    relay_config->server_cfg = NULL;
}