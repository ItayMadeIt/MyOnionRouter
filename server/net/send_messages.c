#include <net_messages.h>

#include <protocol/server_net_structs.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <server.h>

bool send_server_msg(int sock_fd, msg_server_buffer_t* buffer, void* data, uint64_t length)
{
    memset(buffer, 0, sizeof(msg_server_buffer_t));

    memcpy(buffer, data, length);

    int count = send(sock_fd, buffer, sizeof(msg_server_buffer_t), 0);
    if (count != sizeof(msg_server_buffer_t))
    {
        return false;
    }

    return true;
}

bool send_enc_server_msg(int sock_fd, msg_server_buffer_t* buffer, void* data, uint64_t length, key_data_t* key)
{
    memset(buffer, 0, sizeof(msg_server_buffer_t));

    memcpy(buffer, data, length);

    symmetric_encrypt(key, (uint8_t*)buffer, sizeof(msg_server_buffer_t));

    int count = send(sock_fd, buffer, sizeof(msg_server_buffer_t), 0);
    if (count != sizeof(msg_server_buffer_t))
    {
        return false;
    }

    return true;
}


bool send_handshake_response(int sock_fd, msg_server_buffer_t* buffer) 
{
    server_handshake_response_t response;

    response.g = encryption_globals.g;
    memcpy(response.p, encryption_globals.p, sizeof(response.p));
    get_public_identity_key(&server_id_key, response.server_pubkey);

    return send_server_msg(sock_fd, buffer, &response, sizeof(response));
}


bool send_handshake_confirmation(int sock_fd, msg_server_buffer_t* buffer, key_data_t* session_key, uint32_t id) 
{
    server_handshake_confirmation_t confirm;

    memcpy(confirm.magic, SERVER_HANDSHAKE_V1_MAGIC, SERVER_HANDSHAKE_V1_MAGIC_LEN);
    confirm.timestamp = time(NULL);

    confirm.assigned_id = id;

    return send_enc_server_msg(sock_fd, buffer, &confirm, sizeof(confirm), session_key);
}

bool send_enc_relay_signup_response(int sock_fd, key_data_t* key, msg_server_buffer_t *buffer, server_responses_t response_type, uint32_t id)
{
    server_relay_response_signup_t response = {
        .base.request.command=RELAY_COMMAND_SIGNUP,
        .base.timestamp=time(NULL),
        .base.response_code=response_type,
        .id=id
    };

    return send_enc_server_msg(sock_fd, buffer, &response, sizeof(response), key);
}

bool send_enc_relay_signout_response(int sock_fd, key_data_t *key, msg_server_buffer_t *buffer, server_responses_t response_type)
{
    server_relay_response_signout_t response = {
        .base.request.command=RELAY_COMMAND_SIGNOUT,
        .base.timestamp=time(NULL),
        .base.response_code=response_type
    };

    return send_enc_server_msg(sock_fd, buffer, &response, sizeof(response), key);
}

bool send_enc_relay_exit_response(int sock_fd, key_data_t *key, msg_server_buffer_t *buffer, server_responses_t response_type)
{
    server_relay_response_exit_t response = {
        .base.request.command=RELAY_COMMAND_EXIT,
        .base.timestamp=time(NULL),
        .base.response_code=response_type,
    };

    return send_enc_server_msg(sock_fd, buffer, &response, sizeof(response), key);
}

bool send_enc_client_relay_map(int sock_fd, key_data_t *key, msg_server_buffer_t *buffer, server_relay_list_t *list, server_responses_t response_type)
{
    server_client_response_map_t response ={
        .base.request.command=CLIENT_COMMAND_GET_RELAY_MAP,
        .base.timestamp=time(NULL),
        .base.response_code=response_type,
    };

    response.relays.relay_amount = list->relay_amount;
    memcpy(&response.relays.relays, &list->relays, sizeof(relay_descriptor_t) * SERVER_RELAYS_MAP_AMOUNT);

    return send_enc_server_msg(sock_fd, buffer, &response, sizeof(server_client_response_map_t), key);
}

bool send_enc_client_exit(int sock_fd, key_data_t *key, msg_server_buffer_t *buffer, server_responses_t response_type)
{
    server_client_response_exit_t response = {
        .base.request.command=CLIENT_COMMAND_EXIT,
        .base.timestamp=time(NULL),
        .base.response_code=response_type,
    };

    return send_enc_server_msg(sock_fd, buffer, &response, sizeof(response), key);
}
