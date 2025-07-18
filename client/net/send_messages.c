#include "protocol/server_net_structs.h"
#include <net_messages.h>

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

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

bool send_server_handshake_req(int sock_fd, msg_server_buffer_t* buffer)
{
    server_handshake_request_t data = {
        .version = 1,
        .user_type = prot_user_type_relay,
        .flags = 0
    };

    return send_server_msg(sock_fd, buffer, &data, sizeof(server_handshake_request_t));
}

bool send_server_handshake_key(int sock_fd, msg_server_buffer_t* buffer, uint8_t key[ASYMMETRIC_KEY_BYTES])
{
    server_handshake_client_key_t data;
    memcpy(&data.client_pubkey, key, ASYMMETRIC_KEY_BYTES);

    return send_server_msg(sock_fd, buffer, &data, sizeof(server_handshake_client_key_t));
}

bool send_server_relay_map_req(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key)
{
    server_client_request_exit_t data ={
        .base.command = CLIENT_COMMAND_GET_RELAY_MAP,
    };

    return send_enc_server_msg(sock_fd, buffer, &data, sizeof(server_relay_request_exit_t), key);
}

bool send_server_exit_req(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key)
{
    server_client_request_exit_t data ={
        .base.command = CLIENT_COMMAND_EXIT,
    };

    return send_enc_server_msg(sock_fd, buffer, &data, sizeof(server_relay_request_exit_t), key);
}
