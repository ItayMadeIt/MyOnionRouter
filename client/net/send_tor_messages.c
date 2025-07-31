#include "encryptions/encryptions.h"
#include "protocol/tor_structs.h"
#include "tor_net_messages.h"
#include "net_messages.h"
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

bool tor_send_create_msg(client_session_t* session, msg_tor_buffer_t* buffer, const  identity_key_t* id_key)
{    
    msg_tor_create_t* create_msg = (msg_tor_create_t*)buffer;
    create_msg->cmd = TOR_CREATE;
    get_public_identity_key(id_key, create_msg->public_client_key);

    return send_tor_buffer(session->sock_fd, buffer, &session->tls_key, NULL, 0);
}

bool tor_send_extend_msg(client_session_t* session, msg_tor_buffer_t* buffer, const  identity_key_t* id_key, const relay_descriptor_t* relay_descriptor)
{    
    msg_tor_relay_extend_t* msg_extend = (msg_tor_relay_extend_t*)buffer;
    msg_extend->relay = TOR_RELAY;
    msg_extend->cmd = RELAY_EXTEND;
    get_public_identity_key(id_key, msg_extend->client_public_key);
    
    memcpy(&msg_extend->relay_addr, relay_descriptor, sizeof(sock_addr_t));

    return send_tor_buffer(session->sock_fd, buffer,  &session->tls_key, session->onion_keys, session->cur_relays);
}

bool tor_send_begin_msg(client_session_t* session, msg_tor_buffer_t* buffer, uint32_t stream_id, sock_addr_t* addr)
{
    msg_tor_relay_begin_t* begin = (msg_tor_relay_begin_t*)buffer;
    begin->relay = TOR_RELAY;
    begin->cmd = RELAY_BEGIN;
    begin->stream_id = stream_id;

    memcpy(&begin->sock_addr, addr, sizeof(sock_addr_t));

    return send_tor_buffer(session->sock_fd, buffer, &session->tls_key, session->onion_keys, session->cur_relays);
}

bool tor_send_data_msg  (client_session_t* session, msg_tor_buffer_t* buffer, uint32_t stream_id, uint8_t* buf_data, uint16_t length)
{
    msg_tor_relay_data_t* data = (msg_tor_relay_data_t*)buffer;
    data->relay = TOR_RELAY;
    data->cmd = RELAY_DATA;
    data->stream_id = stream_id;

    data->length = length;
    memcpy(data->data, buf_data, data->length);

    return send_tor_buffer(session->sock_fd, buffer, &session->tls_key, session->onion_keys, session->cur_relays);
}

bool tor_send_end_msg  (client_session_t* session, msg_tor_buffer_t* buffer, uint32_t stream_id)
{
    msg_tor_relay_end_t* end = (msg_tor_relay_end_t*)buffer;
    end->relay = TOR_RELAY;
    end->cmd = RELAY_END;
    end->reason = TOR_REASON_END_DONE;
    end->stream_id = stream_id;
    
    return send_tor_buffer(session->sock_fd, buffer, &session->tls_key, session->onion_keys, session->cur_relays);
}
bool tor_send_ping_msg (client_session_t* session, msg_tor_buffer_t* buffer)
{
    msg_tor_padding_t* padding = (msg_tor_padding_t*)buffer;
    padding->cmd = TOR_PADDING;

    return send_tor_buffer(session->sock_fd, buffer, &session->tls_key, session->onion_keys, session->cur_relays);
}

bool tor_send_destroy_msg (client_session_t* session, msg_tor_buffer_t* buffer)
{
    msg_tor_destroy_t* destroy = (msg_tor_destroy_t*)buffer;
    destroy->cmd = TOR_DESTROY;

    return send_tor_buffer(session->sock_fd, buffer, &session->tls_key, session->onion_keys, session->cur_relays);
}