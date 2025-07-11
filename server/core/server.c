#include <server.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <utils/string_utils.h>
#include <utils/sock_utils.h>
#include <utils/server_config.h>
#include <protocol/server_net_structs.h>
#include <encryptions/encryptions.h>

#define CONNECTIONS_BACKLOG_AMOUNT 16
#define INPUT_SIZE 128

static int server_fd;

global_data_t encryption_globals;
identity_key_t server_id_key;

/* 
static void encryption_test()
{
    // Initialize encryption (sets up DH globals)
    init_encryption();

    // Generate Alice and Bob key data
    key_data_t alice = init_key();
    key_data_t bob = init_key();

    // Generate private keys (and compute g^x)
    gen_asymmetric_private_key(&alice);
    gen_asymmetric_private_key(&bob);

    // Export public keys
    uint8_t alice_pub[ASYMMETRIC_KEY_BYTES] = {0};
    uint8_t bob_pub[ASYMMETRIC_KEY_BYTES] = {0};

    get_dh_key_other_public(&alice.asymmetric_key, alice_pub);
    get_dh_key_other_public(&bob.asymmetric_key, bob_pub);

    // Exchange public keys and derive symmetric key
    derive_symmetric_key_from_public(&alice, bob_pub);
    derive_symmetric_key_from_public(&bob, alice_pub);

    // Compare symmetric keys
    if (memcmp(&alice.symmetric_key, &bob.symmetric_key, SYMMETRIC_KEY_BYTES) == 0)
    {
        printf("Shared secret matches!\n");
    }
    else
    {
        printf("Shared secret does NOT match!\n");
    }

    // Cleanup
    free_key(&alice);
    free_key(&bob);
    free_encryption();
}*/

static void run_server_cmd()
{
    bool running = true;

    while (running)
    {
        char input[INPUT_SIZE];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        printf("`%s`\n", input);
    
        if (strcmp(input, "exit") == 0)
        {
            running = false;
        }
    }
}

static void client_callback(int client_fd)
{
    server_handshake_request_t data;
    int read_count = read(client_fd, &data, sizeof(server_handshake_request_t));
    if (read_count == 0)
    {
        // Error occured
        return;
    }

    // Only version 1 is supported
    if (data.version != 1)
    {
        return;
    }

    switch (data.user_type)
    {
    case prot_user_type_client:
        break;
    
    case prot_user_type_relay:
        break;

    default:
        fprintf(stderr, "Client %d has invalid user type: %d\n", client_fd, data.user_type);
        return;
    }

    close(client_fd);

    return;
}

static void* accept_loop_func(void* _)
{
    accept_loop(server_fd, client_callback);

    return NULL;
}

server_code_t run_server(const char* config_filepath)
{
    server_config_metadata_t* config;
    if (fetch_server_config(config_filepath, &config) == false)
    {
        // Invalid path
        return server_error;
    }

    init_encryption();
    get_globals(&encryption_globals.g, encryption_globals.p);
    init_id_key(&server_id_key);

    // Bind
    server_fd = create_and_bind(config);

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

    free_id_key(&server_id_key);
    free_encryption();

    free_server_config(config);

    return server_success;
}