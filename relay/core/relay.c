#include <relay.h>

#include <utils/debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include <session.h>
#include <handle_tls.h>
#include <server_register.h>
#include <protocol/server_net_structs.h>
#include <protocol/tor_structs.h>
#include <net_messages.h>
#include <utils/sock_utils.h>

#define CONNECTIONS_BACKLOG_AMOUNT 16

relay_vars_t relay_vars;

static void client_callback(user_descriptor_t* user)
{
    // Init session variables
    relay_session_t session;
    init_relay_session(&session, user->fd);
    
    // Handle TLS (Get common key between last connection and relay)
    if (handle_tls_recviever(user->fd, &session.tls_last_key) == false)
    {
        session.last_fd = -1;
        free_relay_session(&session);
        free(user);
        printf("Session failed tls\n");
        return;
    }

    relay_code_t code = process_relay_session(&session);
    if (code != relay_success)
    {
        printf("Free user\n");
        free(user);
        return;
    } 

    printf("Free session\n");
    free_relay_session(&session);
    printf("Free user\n");
    free(user);
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