#include <stdio.h>
#include <encryptions/encryptions.h>
#include <string.h>

identity_key_t id_server_key;
key_data_t session_server_key;

identity_key_t id_relay_key;
key_data_t session_relay_key;

int main(int argc, char** argv)
{
    printf("Client\n");

    init_encryption();
    uint64_t g;
    uint8_t p[ASYMMETRIC_KEY_BYTES];


    init_id_key(&id_server_key);
    
    get_globals(&g, p);
    set_globals(g,p);

    init_id_key(&id_relay_key);

    init_key(&session_server_key, &id_server_key);
    init_key(&session_relay_key, &id_relay_key);

    uint8_t relay_public_key[ASYMMETRIC_KEY_BYTES];
    get_public_identity_key(&id_relay_key, relay_public_key);
    
    uint8_t server_public_key[ASYMMETRIC_KEY_BYTES];
    get_public_identity_key(&id_server_key, server_public_key);

    derive_symmetric_key_from_public(&session_relay_key, server_public_key);
    derive_symmetric_key_from_public(&session_server_key,relay_public_key);

    int result = memcmp(&session_relay_key.symmetric_key, &session_server_key.symmetric_key, SYMMETRIC_KEY_BYTES); 
    printf("%d\n", result);

    return 0;
}

