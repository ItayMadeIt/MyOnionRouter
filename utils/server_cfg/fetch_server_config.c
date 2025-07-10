#include "server_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <file_utils/file_utils.h>
#include <string_utils/string_utils.h>

#undef NULL
#define NULL 0

static server_config_metadata_t* str_to_metadata(const char* str)
{
    server_config_metadata_t* result = (server_config_metadata_t*)calloc(1, sizeof(server_config_metadata_t));

    if (result == NULL) return NULL;

    char* mutable_str = clone_str(str);
    uint32_t length = strlen(mutable_str);

    char* mutable_end = mutable_str + length;

    for (uint32_t i = 0; i < length; i++) 
    {
        if (mutable_str[i] == '\n')
        {
            mutable_str[i] = NULL;
        }
    }

    char* iter = mutable_str;
    
    while(iter < mutable_end)
    {
        if (memcmp(iter, "server", sizeof("server")-1) == 0
            && result->server == NULL)
        {
            iter += sizeof("server");

            result->server = (char*)clone_str(iter);
        }
        else if (memcmp(iter, "port", sizeof("port")-1) == 0
            && result->port == NULL)
        {
            iter += sizeof("port");

            result->port = (char*)clone_str(iter);
        }

        iter += strlen(iter) + 1;
    }

    free(mutable_str);

    return result;
}

bool fetch_server_config(const char* filepath, server_config_metadata_t** metadata)
{
    FILE* config_fd = fopen(filepath, "r");
    if (config_fd == NULL)
    {
        fprintf(stderr, "config_file doesn't exist\n");
        return false;
    }

    char* config_str = file_to_string(config_fd);
    fclose(config_fd);

    if (config_str == NULL)
    {
        return false;
    }

    *metadata = str_to_metadata(config_str);
    free(config_str);

    return true;
}