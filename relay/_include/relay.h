#ifndef __RELAY_H__
#define __RELAY_H__

#include "encryptions/encryptions.h"
#include <relay_config.h>

typedef enum relay_code
{
    relay_success = 0,
    relay_error   = 1,
    relay_unexpected_msg = 2,
    relay_conntinue = ~0,
} relay_code_t;

typedef struct relay_vars
{
    identity_key_t key;
    key_data_t server_key;
    uint32_t register_id;
    bool is_key_init;
    const relay_config_metadata_t* config;
    int sock_fd;
} relay_vars_t;

typedef struct relay_session 
{
    key_data_t onion_key;
    key_data_t tls_last_key;
    key_data_t tls_next_key;
    int last_fd;
    int next_fd;

} relay_session_t;

extern relay_vars_t relay_vars;

relay_code_t run_relay(const relay_config_metadata_t* relay_config);

#endif // __RELAY_H__