#include "protocol/relay_data_structs.h"
#include <server_query.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include <encryptions/encryptions.h>
#include <protocol/server_net_structs.h>
#include <client.h>
#include <net_messages.h>
#include <utils/sock_utils.h>

static client_code_t handle_handshake(int sock_fd, msg_server_buffer_t* buffer, key_data_t* result_server_key, identity_key_t* result_id_key)
{
    if (send_server_handshake_req(sock_fd, buffer) == false)
    {
        return client_error;
    }

    server_handshake_response_t handshake_response;
    if (recv_server_handshake_res(sock_fd, buffer, &handshake_response) == false)
    {
        return client_error;
    }

    // Setup identity key
    set_globals(handshake_response.g, handshake_response.p);

    init_id_key(result_id_key);

    // Setup a server key
    init_key(result_server_key, result_id_key);
    derive_symmetric_key_from_public(result_server_key, handshake_response.server_pubkey);

    uint8_t public_key[ASYMMETRIC_KEY_BYTES];
    get_public_identity_key(result_id_key, public_key);

    if (send_server_handshake_key(sock_fd, buffer, public_key) == false)
    {
        return client_error;
    }

    server_handshake_confirmation_t confirmation;
    if (recv_server_handshake_confirmation(sock_fd, buffer, result_server_key, &confirmation) == false)
    {
        return client_error;
    }

    if (memcmp(SERVER_HANDSHAKE_V1_MAGIC, confirmation.magic, SERVER_HANDSHAKE_V1_MAGIC_LEN) != 0)
    {
        fprintf(stderr, "MAGIC V1 value didn't match: %s\n", confirmation.magic);

        return client_error;
    }

    return client_success;
}

static client_code_t fetch_relay_map(int sock_fd, msg_server_buffer_t* buffer, key_data_t* server_key, server_relay_list_t* list)
{
    if (send_server_relay_map_req(sock_fd, buffer, server_key) == false)
    {
        return client_error;
    }

    if (recv_server_relay_map_res(sock_fd, buffer, server_key, list) == false)
    {
        return client_error;
    }

    return client_success;
}

static client_code_t handle_exit(int sock_fd, msg_server_buffer_t* buffer, key_data_t* server_key)
{
    if (send_server_exit_req(sock_fd, buffer, server_key) == false)
    {
        return client_error;
    }

    if (recv_server_exit_res(sock_fd, buffer, server_key) == false)
    {
        return client_error;
    }

    return client_success;
}

client_code_t gather_relay_map(circuit_relay_list_t* relay_list, uint8_t relay_amount)
{
    msg_server_buffer_t buffer;

    int sock_fd = connect_server(&client_vars.config->server_cfg);

    if (sock_fd == -1)
    {
        return client_error;
    }

    key_data_t server_key;
    
    client_code_t response = handle_handshake(sock_fd, &buffer, &server_key, &client_vars.id_key);
    if (response != client_success)
    {
        close(sock_fd);
        free_key(&server_key);
        return response;
    }

    server_relay_list_t server_list;
    response = fetch_relay_map(sock_fd, &buffer, &server_key, &server_list);
    if (response != client_success)
    {
        close(sock_fd);
        free_key(&server_key);
        return response;
    }

    if (server_list.relay_amount < relay_amount)
    {   
        close(sock_fd);
        free_key(&server_key);
        return client_error;
    }

    relay_list->relay_amount = relay_amount;
    memcpy(relay_list->relays, &server_list.relays, relay_amount * sizeof(relay_descriptor_t));

    response = handle_exit(sock_fd, &buffer, &server_key);

    close(sock_fd);
    free_key(&server_key);

    return response;
}