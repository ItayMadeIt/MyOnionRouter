#include "handle_tls.h"

#include <protocol/tor_structs.h>
#include <net_messages.h>

#include <unistd.h>

bool handle_tls_sender(int sock_fd, key_data_t* this)
{
    tls_key_buffer_t tls_other_buffer;
    if (recv_tls_msg(sock_fd, &tls_other_buffer) == false)
    {
        close(sock_fd);
        return false;
    }
    derive_symmetric_key_from_public(this, (uint8_t*)tls_other_buffer.public_key);

    tls_key_buffer_t tls_this_buffer;
    get_public_identity_key(this->identity, (uint8_t*)tls_this_buffer.public_key);
    if (send_tls_msg(sock_fd, &tls_this_buffer) == false)
    {
        close(sock_fd);
        return false;
    }
    
    return true;
}

bool handle_tls_recviever(int sock_fd, key_data_t* this)
{
    tls_key_buffer_t tls_this_buffer;
    get_public_identity_key(this->identity, (uint8_t*)tls_this_buffer.public_key);
    if (send_tls_msg(sock_fd, &tls_this_buffer) == false)
    {
        close(sock_fd);
        return false;
    }

    tls_key_buffer_t tls_other_buffer;
    if (recv_tls_msg(sock_fd, &tls_other_buffer) == false)
    {
        close(sock_fd);
        return false;
    }
    derive_symmetric_key_from_public(this, (uint8_t*)tls_other_buffer.public_key);

    return true;
}
