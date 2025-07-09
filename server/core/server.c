#include <server.h>

#include <utils/string_utils.h>
#include <encryptions/encryptions.h>

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sock_utils.h>

#define CONNECTIONS_BACKLOG_AMOUNT 16
#define INPUT_SIZE 128

static int server_fd;

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
}

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
    close(client_fd);

    return;
}

static void* accept_loop_func(void* _)
{
    accept_loop(server_fd, client_callback);

    return NULL;
}

server_code_t run_server(const char* config_fliepath)
{
    config_metadata_t config;
    config.port = clone_str("8200");
    config.server = clone_str("mor.qk");
    /*
    if (fetch_config(config_fliepath, &config) == false)
    {
        fprintf(stderr, "Invalid path.");
        return server_error;
    }*/

    // Bind
    server_fd = create_and_bind(&config);
    // Listen
    listen(server_fd, CONNECTIONS_BACKLOG_AMOUNT);

    // Handle connections
    pthread_t conn_thread_id;
    pthread_create(
        &conn_thread_id, 
        NULL, 
        accept_loop_func,
        NULL
    );

    run_server_cmd();

    pthread_detach(conn_thread_id);

    free_config(&config);

    return server_success;
}