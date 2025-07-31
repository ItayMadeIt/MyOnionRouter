#include <net_messages.h>
#include <protocol/server_net_structs.h>
#include <utils/debug.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool recv_tls_msg(int sock_fd, tls_key_buffer_t *data) 
{ 
    int count = read(sock_fd, data, sizeof(tls_key_buffer_t));

    if (count <= 0)
    {
        return false;
    }
    
    return true;
}

bool recv_tor_buffer(int sock_fd, msg_tor_buffer_t* data, key_data_t* tls_key, key_data_t* onion_key, uint8_t onion_amount)
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
    
    if (onion_key && onion_amount > 0)
    {
        // no need to encrypt/decrypt: uint16_t circ_id, uint8_t cmd
        for (uint8_t i = 0; i < onion_amount; i++)
        {
            #ifdef DEBUG
            printf("Decrypt onion %d ", i);print_block(&onion_key[i].symmetric_key);
            #endif
            
            symmetric_decrypt(
                &onion_key[i], 
                (uint8_t*)data + sizeof(uint16_t) + sizeof(uint8_t), 
                sizeof(msg_tor_buffer_t) - sizeof(uint16_t) - sizeof(uint8_t)
            );
        }
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

bool recv_server_handshake_confirmation(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key, server_handshake_confirmation_t* confirmation)
{
    if (recv_enc_server_msg(sock_fd, buffer, key) == false)
    {
        return false;
    }

    *confirmation = *(server_handshake_confirmation_t*)buffer; 

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

bool recv_server_relay_map_res(int sock_fd, msg_server_buffer_t* buffer, key_data_t* key, server_relay_list_t* list)
{
    if (recv_enc_server_msg(sock_fd, buffer, key) == false)
    {
        return false;
    }

    server_client_response_map_t* response = (server_client_response_map_t*)buffer;

    list->relay_amount = response->relays.relay_amount;
    memcpy(&list->relays, &response->relays.relays, sizeof(relay_descriptor_t) * SERVER_RELAYS_MAP_AMOUNT);

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