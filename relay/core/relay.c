#include <relay.h>

#include <unistd.h>

#include <protocol/server_net_structs.h>
#include <utils/sock_utils.h>

static int sock_fd; 

void signup_server()
{
    server_handshake_request_t data = {
        .version = 1,
        .user_type = prot_user_type_relay,
        .flags = 0
    };

    write(sock_fd, &data, sizeof(server_handshake_request_t));
}

relay_code_t run_relay(const relay_config_metadata_t* relay_config)
{
    // Connect to server and sign it up
    sock_fd = connect_server(relay_config->server_cfg);

    if (sock_fd == -1)
    {
        return relay_error;
    }

    signup_server();
    
    // Disconnect from dir server
    close(sock_fd);
    sock_fd = -1;

    // Bind as a server

    // Each client get it's own TOR socket, seperate logic entirely

    return relay_success;
}