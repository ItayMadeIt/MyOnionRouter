#include <server_register.h>

#include <relay.h>
#include <net_messages.h>
#include <protocol/server_net_structs.h>
#include <stdio.h>
#include <string.h>
#include <utils/sock_utils.h>

#include <unistd.h>

relay_code_t signup_server()
{
    msg_server_buffer_t buffer;

    if (relay_vars.register_id != INVALID_RELAY_ID)
    {
        return relay_error;
    }

    int sock_fd = connect_server(relay_vars.config->server_cfg);

    if (sock_fd == -1)
    {
        return relay_error;
    }

    if (send_server_handshake_req(sock_fd, &buffer) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    server_handshake_response_t handshake_response;
    if (recv_server_handshake_res(sock_fd, &buffer, &handshake_response) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    // Setup identity key
    set_globals(handshake_response.g, handshake_response.p);
    init_id_key(&relay_vars.key);

    // Setup a server key
    key_data_t server_key;
    init_key(&server_key, &relay_vars.key);
    derive_symmetric_key_from_public(&server_key, handshake_response.server_pubkey);

    uint8_t public_key[ASYMMETRIC_KEY_BYTES];
    get_public_identity_key(&relay_vars.key, public_key);

    if (send_server_handshake_key(sock_fd, &buffer, public_key) == false)
    {
        free_key(&server_key);
        close(sock_fd);
        return relay_error;
    }

    server_handshake_confirmation_t confirmation;
    if (recv_server_handshake_confirmation(sock_fd, &buffer, &server_key, &confirmation) == false)
    {
        free_key(&server_key);
        close(sock_fd);
        return relay_error;
    }

    if (memcmp(SERVER_HANDSHAKE_V1_MAGIC, confirmation.magic, SERVER_HANDSHAKE_V1_MAGIC_LEN) != 0)
    {
        fprintf(stderr, "MAGIC V1 value didn't match: %s\n", confirmation.magic);

        free_key(&server_key);
        close(sock_fd);

        return relay_error;
    }

    // After that will sign it up using server commands (SIGNUP CMD)

    free_key(&server_key);
    close(sock_fd);

    return relay_success;
}