#include <stdio.h>
#include <server.h>
#include <encryptions/encryptions.h>

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
    if (memcmp(&alice.symmetric_key, &bob.symmetric_key, AES_BYTES) == 0)
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

    
    return 0;
}