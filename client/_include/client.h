#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <protocol/server_net_structs.h>
#include <encryptions/encryptions.h>

#include "client_config.h"

#define MAX_RELAYS 5

// The result circuit relay list, not the one we get from the server.
typedef struct circuit_relay_list
{
    uint8_t relay_amount;
    relay_descriptor_t relays[MAX_RELAYS];
} circuit_relay_list_t;

typedef enum client_code
{
    client_success   = 0,
    client_error     = 1,
    client_tls_error = 2,
    client_continue  = ~0,
} client_code_t;

typedef struct client_vars 
{
    identity_key_t id_key;   
    circuit_relay_list_t circuit_relays;
    const client_config_metadata_t* config; 
    
    // much more variables, socket to first relay, all relay keys, etc...
} client_vars_t;

extern client_vars_t client_vars;

client_code_t run_client();
#endif // __CLIENT_H__