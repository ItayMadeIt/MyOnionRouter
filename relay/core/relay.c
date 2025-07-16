#include <relay.h>

#include <unistd.h>

#include <server_register.h>
#include <protocol/server_net_structs.h>
#include <utils/sock_utils.h>

relay_vars_t relay_vars;

relay_code_t run_relay(const relay_config_metadata_t* relay_config)
{
    relay_vars.config = relay_config;
    
    relay_vars.register_id = INVALID_RELAY_ID;

    init_encryption();

    relay_code_t result;
    result = signup_server();
    if (result != relay_success)
    {
        return result;
    }
    
    // Bind as a server

    // Each client get it's own TOR socket, seperate logic entirely

    return relay_success;
}