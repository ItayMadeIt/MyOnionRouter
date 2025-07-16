#include <net_messages.h>
#include <protocol/server_net_structs.h>
#include <sys/socket.h>
#include <unistd.h>
#include <relay.h>

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