#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <encryptions/encryptions.h>

#include <client_config.h>

typedef enum client_code
{
    client_success = 0,
    client_error   = 1,
} client_code_t;

typedef struct client_vars 
{
    identity_key_t id_key;    
} client_vars_t;

extern client_vars_t client_vars;

client_code_t run_client(const client_config_metadata_t* client_config);
#endif // __CLIENT_H__