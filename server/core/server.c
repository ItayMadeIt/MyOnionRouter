#include <server.h>

#include <config.h>
#include <stdio.h>
#include <stdbool.h>

server_code_t run_server(const char* config_fliepath)
{
    config_metadata_t config;
    if (fetch_config(config_fliepath, &config) == false)
    {
        fprintf(stderr, "Invalid path.");
        return server_error;
    }
    


    return server_success;
}
