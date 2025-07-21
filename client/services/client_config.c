#include "client.h"
#include <client_config.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <utils/string_utils.h>

#define DEAULT_RELAY_AMOUNT 3

bool parse_args(const int argc, const char** argv, client_config_metadata_t* client_config)
{
    if (client_config == NULL)
    {
        return false;
    }

    bool relays_set = false;
    bool server_cfg_set = false;

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp("-r", argv[i]) == 0 || strcmp("--relays", argv[i]) == 0)
            && i + 1 < argc)
        {
            i++;

            client_config->relays = atoi(argv[i]);

            if (client_config->relays <= 0 || client_config->relays > MAX_RELAYS)
            {
                printf("[Warning] - invalid amount of relays %d.", client_config->relays);
                continue;
            }

            relays_set = true;
        }
        else if (server_cfg_set == false && argv[i][0] != '-' && 
            fetch_server_config(argv[i], &client_config->server_cfg))
        {
            server_cfg_set = true;
        }
        else
        {
            return false;
        }
    }

    if (relays_set == false)
    {
        client_config->relays = DEAULT_RELAY_AMOUNT;
    }

    return server_cfg_set;
}