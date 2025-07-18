#include <relay.h>

#include <sys/socket.h>
#include <unistd.h>

#include <server_register.h>
#include <protocol/server_net_structs.h>
#include <utils/sock_utils.h>

relay_vars_t relay_vars;

relay_code_t run_relay(const relay_config_metadata_t* relay_config)
{
    relay_vars.config = relay_config;
    relay_vars.is_key_init = false;
    
    // Bind as a server
    struct sockaddr_storage sockaddr;
    relay_vars.sock_fd = create_and_bind(&relay_config->relay_server_cfg, &sockaddr);

    relay_vars.register_id = INVALID_RELAY_ID;

    init_encryption();

    relay_code_t result;
    result = signup_server(&sockaddr);
    if (result != relay_success)
    {
        return result;
    }
    
    sleep(60);
    // Each client get it's own TOR socket, seperate logic entirely


    result = signout_server();
    if (result != relay_success)
    {
        return result;
    }

    free_key(&relay_vars.server_key);
    close(relay_vars.sock_fd);

    return relay_success;
}