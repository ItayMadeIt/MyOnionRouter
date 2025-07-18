#include <netinet/in.h>
#include <server.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <relay_manager.h>
#include <utils/sock_utils.h>
#include <utils/string_utils.h>
#include <protocol/server_net_structs.h>
#include <encryptions/encryptions.h>
#include <net_messages.h>
#include <socket_context.h>
#include <handlers.h>

#define CONNECTIONS_BACKLOG_AMOUNT 16
#define INPUT_SIZE 128

const server_handshake_confirmation_t base_confirmation = {SERVER_HANDSHAKE_V1_MAGIC, 0, 0};

static int server_fd;

global_data_t encryption_globals;
identity_key_t server_id_key;

static void run_server_cmd()
{
    bool running = true;

    while (running)
    {
        char input[INPUT_SIZE];
        if (fgets(input, sizeof(input), stdin) != input)
        {
            fprintf(stderr, "Invalid input");
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        printf("`%s`\n", input);
    
        if (strcmp(input, "exit") == 0)
        {
            running = false;
        }
    }
}

static void client_callback(user_descriptor_t* user)
{
    msg_server_buffer_t buffer_data;
    key_data_t session_key;

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
        case prot_user_type_relay:
        {
            process_relay(user->fd, &buffer_data);
            break;
        }
        case prot_user_type_client:
        {
            process_client(user->fd, &buffer_data);
            break;
        }
        default:
        {
            fprintf(stderr, "Unknown user type: %d\n", request.user_type);
            break;
        }
    }

    free(user);

    return;
}

static void* accept_loop_func(void* _)
{   
    (void)(_);

    accept_loop(server_fd, client_callback);

    return NULL;
}

server_code_t run_server(const char* config_filepath)
{
    server_config_metadata_t* config;
    if (fetch_server_config(config_filepath, &config) == false)
    {
        // Invalid path
        printf("Invalid config file\n");
        return server_error;
    }

    init_encryption();
    get_globals(&encryption_globals.g, encryption_globals.p);
    init_id_key(&server_id_key);

    init_relay_manager();
    init_socket_context();

    printf("Create and bind:\n");
    // Bind
    server_fd = create_and_bind(config, NULL);
    if (server_fd == -1)
    {
        printf("Failed to bind on %s:%s", config->server, config->port);
    }

    // Listen
    listen(server_fd, CONNECTIONS_BACKLOG_AMOUNT);

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
        return server_error;
    }

    pthread_detach(conn_thread_id);

    run_server_cmd();

    free_socket_context();
    free_relay_manager();

    free_id_key(&server_id_key);
    free_encryption();

    free_server_config(config);

    return server_success;
}