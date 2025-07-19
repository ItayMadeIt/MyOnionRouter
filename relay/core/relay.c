#include <relay.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include <server_register.h>
#include <protocol/server_net_structs.h>
#include <protocol/tor_structs.h>
#include <utils/sock_utils.h>

#define CONNECTIONS_BACKLOG_AMOUNT 16

relay_vars_t relay_vars;


static void client_callback(user_descriptor_t* user)
{
    msg_tor_buffer_t buffer_data;
    key_data_t session_key;

    printf("CLIENT CONNECTED\n\n");

    free(user);

    return;
    
    /*
    init_key(&session_key, &server_id_key);

    server_handshake_request_t request;
    if (recv_handshake_request(user->fd, &buffer_data, &request) == false)
    {
        close(user->fd);
        return;
    }

    if (send_handshake_response(user->fd, &buffer_data) == false)
    {
        close(user->fd);
        return;
    }

    server_handshake_client_key_t client_key_msg;
    if (recv_handshake_client_key(user->fd, &buffer_data, &client_key_msg) == false)
    {
        close(user->fd);
        return;
    }

    derive_symmetric_key_from_public(&session_key, (uint8_t*)client_key_msg.client_pubkey);

    uint32_t id = user->fd;
    
    if (send_handshake_confirmation(user->fd, &buffer_data, &session_key, id) == false)
    {
        close(user->fd);
        return;
    }
    
    add_socket(user->fd, &session_key, NULL);
    
    switch (request.user_type)
    {
        case prot_user_type_client:
        {
            process_client(user->fd, &buffer_data);
            break;
        }
        case prot_user_type_relay:
        {
            process_relay(user->fd, &buffer_data);
            break;
        }
        default:
        {
            fprintf(stderr, "Unknown user type: %d\n", request.user_type);
            break;
        }
    }*/

}

static void* accept_loop_func(void* _)
{   
    (void)(_);

    accept_loop(relay_vars.sock_fd, client_callback);

    return NULL;
}

relay_code_t run_relay(const relay_config_metadata_t* relay_config)
{
    relay_vars.config = relay_config;
    relay_vars.is_key_init = false;
    
    // Bind as a server
    relay_vars.sock_fd = create_and_bind(&relay_config->relay_server_cfg);

    relay_vars.register_id = INVALID_RELAY_ID;

    init_encryption();

    relay_code_t result;
    result = signup_server();
    if (result != relay_success)
    {
        return result;
    }

    listen(relay_vars.sock_fd, CONNECTIONS_BACKLOG_AMOUNT);
    
    // Handle connections
    pthread_t conn_thread_id;
    int conn_thread_err = pthread_create(
        &conn_thread_id, 
        NULL, 
        accept_loop_func,
        NULL
    );

    if (conn_thread_err)
    {
        // Invalid thread creation
        return relay_error;
    }
    pthread_detach(conn_thread_err);

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