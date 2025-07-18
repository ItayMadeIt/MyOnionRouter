#include <client.h>

#include <client_config.h>

#include <stdio.h>
#include <stdlib.h>

#define MIN_ARGS 2
#define MAX_ARGS 4

client_vars_t client_vars;

int main(int argc, const char** argv)
{
    if (MIN_ARGS > argc || argc > MAX_ARGS)
    {
        fprintf(stderr, "Usage: client <config_file> [-r <relays> | --relays <relays>]\n");
        return -1;
    }

    // Parse args into a client_config
    client_config_metadata_t* client_config;
    parse_args(argc, argv, &client_config);
    if (client_config == NULL)
    {
        fprintf(stderr, "Usage: client <config_file> [-r <relays> | --relays <relays>]\n");
        return -1;
    }

    client_vars.config = client_config;

    client_code_t code = run_client();

    free(client_config);
    client_config = NULL;

    if (code != client_success)
    {
        fprintf(stderr, "Client failed\n");
        return -1;
    }

    return 0;
}

