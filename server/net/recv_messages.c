#include <net_messages.h>
#include <protocol/server_net_structs.h>
#include <sys/socket.h>
#include <unistd.h>
#include <server.h>

bool recv_server_msg(int sock_fd, msg_buffer_t* buffer)
{
    int count = read(sock_fd, buffer, sizeof(msg_buffer_t));

    if (count <= 0)
    {
        return false;
    }
    
    return true;
}

bool recv_enc_server_msg(int sock_fd, msg_buffer_t* buffer, key_data_t* key)
{
    int count = read(sock_fd, buffer, sizeof(msg_buffer_t));

    if (count <= 0)
    {
        return false;
    }

    symmetric_decrypt(key, (uint8_t*)buffer, sizeof(msg_buffer_t)); 
    
    return true;
}

bool recv_handshake_request(int sock_fd, msg_buffer_t* buffer, server_handshake_request_t* req) 
{
    if (recv_server_msg(sock_fd, buffer) == false) 
    {
        return false;
    }

    *req = *(server_handshake_request_t*)buffer;
    return (req->version == 1 &&
            (req->user_type == prot_user_type_client || req->user_type == prot_user_type_relay));
}

bool recv_handshake_client_key(int sock_fd, msg_buffer_t* buffer, server_handshake_client_key_t* client_key) 
{
    if (recv_server_msg(sock_fd, buffer) == false) 
    {
        return false;
    }

    *client_key = *(server_handshake_client_key_t*)buffer;
    return true;
}