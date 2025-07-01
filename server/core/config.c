#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#undef NULL
#define NULL 0

#define ARGS 2

char* strdup(const char* str);

char* file_to_string(FILE* fd)
{
    fseek(fd, 0L, SEEK_END);
    int length = ftell(fd);

    char* str = malloc(length + 1);
    str[length] = '\0'; 

    fseek(fd, 0, SEEK_SET);

    int bytes = 0;
    if ((bytes = fread(str, sizeof(char), length, fd)) != length)
    {
        fprintf(stderr, "Bytes was not sufficient in it's mission to read bytes.\n");
        return NULL;
    }

    return str;
}

config_metadata_t str_to_metadata(const char* str)
{
    config_metadata_t result;
    memset(&result, NULL, sizeof(config_metadata_t));

    char* mutable_str = (char*)strdup(str);
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
            && result.server == NULL)
        {
            iter += sizeof("server");

            result.server = (char*)strdup(iter);
        }
        else if (memcmp(iter, "port", sizeof("port")-1) == 0)
        {
            iter += sizeof("port");

            result.port = atoi(iter);
        }
        else if (memcmp(iter, "relays", sizeof("relays")-1) == 0)
        {
            iter += sizeof("relays");

            result.relays = atoi(iter);
        }

        iter += strlen(iter) + 1;
    }

    free(mutable_str);

    return result;
}

void free_config(config_metadata_t* metadata)
{
    free(metadata->server);
}

bool fetch_config(const char* filepath, config_metadata_t* metadata)
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