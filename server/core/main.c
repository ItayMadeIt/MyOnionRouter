#include <stdio.h>
#include <server.h>
#include <dh.h>

#include <inttypes.h>


#define ARGS 2

int main(int argc, char *argv[])
{
    if (argc != ARGS)
    {
        fprintf(stderr, "Use: server <config_file>\n");
        return -1;
    }
    
    server_code_t code = run_server(argv[1]);
    
    if (code != server_success)
    {
        fprintf(stderr, "Server failed");
        return 1;
    }

     // initialize dh (generates n, g)
    init_dh();

    // Create two keys alice & bob
    dh_key_t alice = create_dh_key();
    dh_key_t bob = create_dh_key();

    // export public keys
    uint8_t alice_pub[DH_KEY_BYTES] = {0};
    uint8_t bob_pub[DH_KEY_BYTES] = {0};

    export_other_public(&alice, alice_pub);
    export_other_public(&bob, bob_pub);

    // Set the other's public key and compute common key
    set_dh_key_other_public(&alice, bob_pub);
    set_dh_key_other_public(&bob, alice_pub);

    // Now alice.common_key and bob.common_key should be equal
    if (mpz_cmp(alice.common_key, bob.common_key) == 0) {
        printf("Shared secret matches!\n");
    } else {
        printf("Shared secret does NOT match!\n");
    }

    // Clean up
    free_dh_key(&alice);
    free_dh_key(&bob);
    free_dh();

    return 0;
}