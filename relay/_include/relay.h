#ifndef __RELAY_H__
#define __RELAY_H__

#include <relay_config.h>

typedef enum relay_code
{
    relay_success = 0,
    relay_error   = 1,
} relay_code_t;

relay_code_t run_relay(const relay_config_metadata_t* relay_config);

#endif // __RELAY_H__