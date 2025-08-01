#include "protocol/tor_structs.h"
#include "relay.h"
#include "session.h"

bool tor_send_end_relay_end(relay_session_t* session, msg_tor_buffer_t* buffer, uint16_t stream_id, tor_relay_end_reasons_t reason);
bool tor_send_end_relay_data(relay_session_t* session, msg_tor_buffer_t* buffer, uint16_t stream_id, uint8_t* data, uint16_t length);
bool tor_send_end_relay_connected(relay_session_t* session, msg_tor_buffer_t* buffer, uint16_t stream_id);
bool tor_send_destroy_msg(relay_session_t* session, msg_tor_buffer_t* buffer, bool to_client);

bool tor_send_middle_relay_create(relay_session_t* session, msg_tor_buffer_t* buffer, uint8_t* client_public_key);
bool tor_send_middle_relay_extended(relay_session_t* session, msg_tor_buffer_t* buffer);

void tor_parse_end_msg      (msg_tor_buffer_t* buffer, uint32_t* stream_id, tor_relay_end_reasons_t* reason);
