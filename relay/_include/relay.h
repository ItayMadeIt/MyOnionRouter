#ifndef __RELAY_H__
#define __RELAY_H__

#include "encryptions/encryptions.h"
#include <relay_config.h>

typedef enum relay_code
{
    relay_success = 0,
    relay_error   = 1,
} relay_code_t;

typedef struct relay_vars
{
    identity_key_t key;
    uint32_t register_id;
    const relay_config_metadata_t* config;

} relay_vars_t;

extern relay_vars_t relay_vars;

relay_code_t run_relay(const relay_config_metadata_t* relay_config);

#endif // __RELAY_H__