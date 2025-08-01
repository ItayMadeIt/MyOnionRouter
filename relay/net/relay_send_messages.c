#include "relay_messages.h"
#include "net_messages.h"
#include <string.h>

bool tor_send_end_relay_end(relay_session_t* session, msg_tor_buffer_t* buffer, uint16_t stream_id, tor_relay_end_reasons_t reason)
{
    msg_tor_relay_end_t* msg_end = (msg_tor_relay_end_t*)buffer;
    msg_end->relay = TOR_RELAY;
    msg_end->stream_id = stream_id;
    msg_end->cmd = RELAY_END;
    
    msg_end->reason = reason;

    return send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true);
}

bool tor_send_end_relay_data(relay_session_t *session, msg_tor_buffer_t *buffer, uint16_t stream_id, uint8_t *data, uint16_t length)
{
    msg_tor_relay_end_t* msg_data = (msg_tor_relay_end_t*)buffer;
    msg_data->relay = TOR_RELAY;
    msg_data->stream_id = stream_id;
    msg_data->cmd = RELAY_DATA;

    msg_data->length = length;
    memcpy(msg_data->data, data, length);

    return send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true);
}

bool tor_send_end_relay_connected(relay_session_t* session, msg_tor_buffer_t* buffer, uint16_t stream_id)
{
    msg_tor_relay_connected_t* msg_connected = (msg_tor_relay_connected_t*)buffer;
    msg_connected->relay = TOR_RELAY;
    msg_connected->stream_id = stream_id;
    msg_connected->cmd = RELAY_CONNECTED;

    return send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true);
}

bool tor_send_middle_relay_create(relay_session_t* session, msg_tor_buffer_t* buffer, uint8_t* client_public_key)
{
    msg_tor_create_t* create = (msg_tor_create_t*)buffer;
    create->cmd = TOR_CREATE;
    memmove(create->public_client_key, client_public_key, ASYMMETRIC_KEY_BYTES);

    return send_tor_buffer(session->next_fd, buffer, &session->tls_next_key, NULL, false);
}

bool tor_send_middle_relay_extended(relay_session_t* session, msg_tor_buffer_t* buffer)
{
    msg_tor_relay_extended_t* extended = (msg_tor_relay_extended_t*)buffer;
    extended->relay = TOR_RELAY;
    extended->cmd = RELAY_EXTENDED;

    return send_tor_buffer(session->last_fd, buffer, &session->tls_last_key, &session->onion_key, true);
}
