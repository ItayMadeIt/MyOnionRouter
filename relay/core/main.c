#include <stdio.h>
#include <relay.h>
#include <configs.h>
#include <stdlib.h>
#include <utils/string_utils.h>


#define MAX_ARGS 4
#define MIN_ARGS 2

int main(int argc, char *argv[])
{
    // Ensure args
    if (MIN_ARGS > argc || argc > MAX_ARGS)
    {
        fprintf(stderr, "Usage: relay <config_file> [-p <port> | --port <port>]\n");
        return -1;
    }

    // Parse args into a relay_config_metadata_t
    relay_config_metadata_t* relay_config;
    parse_args(argc-1, argv+1, &relay_config);
    if (relay_config == NULL)
    {
        fprintf(stderr, "Usage: relay <config_file> [-p <port> | --port <port>]\n");
        return -1;
    }
    
    relay_code_t code = run_relay(relay_config);
    
    free(relay_config);
    relay_config = NULL;

    if (code != relay_success)
    {
        fprintf(stderr, "Relay failed");
        return -1;
    }
    
    return 0;
}