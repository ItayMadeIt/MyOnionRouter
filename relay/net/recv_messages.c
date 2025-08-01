#include "protocol/tor_structs.h"
#include <net_messages.h>
#include <protocol/server_net_structs.h>
#include <sys/socket.h>
#include <unistd.h>
#include <relay.h>

#include <utils/debug.h>

bool recv_tls_msg(int sock_fd, tls_key_buffer_t *data) 
{ 
    int count = read(sock_fd, data, sizeof(tls_key_buffer_t));

    if (count <= 0)
    {
        return false;
    }
    
    return true;
}

bool recv_tor_buffer(int sock_fd, msg_tor_buffer_t* data, key_data_t* tls_key, key_data_t* onion_key, bool from_client)
{
    int count = read(sock_fd, data, sizeof(msg_tor_buffer_t));

    if (count <= 0)
    {
        return false;
    }

    if (tls_key)
    {
        #ifdef DEBUG
        printf("Decrypt tls "); print_block(&tls_key->symmetric_key);
        #endif

        symmetric_decrypt(tls_key, (uint8_t*)data, sizeof(msg_tor_buffer_t));
    }

    // no need to encrypt/decrypt: uint16_t circ_id, uint8_t cmd
    if (onion_key && from_client)
    {
        #ifdef DEBUG
        printf("Decrypt "); print_block(&onion_key->symmetric_key);
        #endif
        symmetric_decrypt(
            onion_key, 
            (uint8_t*)data + sizeof(uint8_t) + sizeof(uint16_t), 
            sizeof(msg_tor_buffer_t) - sizeof(uint8_t) - sizeof(uint16_t)
        );
    }

#ifdef DEBUG
    msg_tor_t* tor = (msg_tor_t* )data;
    printf("Recveied CMD: %d\n", tor->cmd); 
#endif

    return true;
}

bool recv_server_msg(int sock_fd, msg_server_buffer_t* buffer)
{
    int count = read(sock_fd, buffer, sizeof(msg_server_buffer_t));

    if (count <= 0)
    {
        return false;
    }
    
    return true;
}

bool recv_enc_server_msg(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key)
{
    int count = read(sock_fd, buffer, sizeof(msg_server_buffer_t));

    if (count <= 0)
    {
        return false;
    }

    symmetric_decrypt(key, (uint8_t*)buffer, sizeof(msg_server_buffer_t)); 
    
    return true;
}

bool recv_server_handshake_res(int sock_fd, msg_server_buffer_t* buffer, server_handshake_response_t* response)
{
    if (recv_server_msg(sock_fd, buffer) == false)
    {
        return false;
    }

    *response = *(server_handshake_response_t*)buffer; 

    return true;
}

bool recv_server_handshake_confirmation(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key, server_handshake_confirmation_t* confirmation)
{
    if (recv_enc_server_msg(sock_fd, buffer, key) == false)
    {
        return false;
    }

    *confirmation = *(server_handshake_confirmation_t*)buffer; 

    return true;
}

bool recv_server_signup_res(int sock_fd, msg_server_buffer_t *buffer, key_data_t *key, uint32_t *id)
{
    if (recv_enc_server_msg(sock_fd, buffer, key) == false)
    {
        return false;
    }

    server_relay_response_signup_t* response = (server_relay_response_signup_t*)buffer;

    *id = response->id; 

    return true;
}

bool recv_server_signout_res(int sock_fd, msg_server_buffer_t *buffer, key_data_t *key)
{
    if (recv_enc_server_msg(sock_fd, buffer, key) == false)
    {
        return false;
    } 

    return true;
}

bool recv_server_exit_res(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key)
{
    if (recv_enc_server_msg(sock_fd, buffer, key) == false)
    {
        return false;
    }

    return true;
}