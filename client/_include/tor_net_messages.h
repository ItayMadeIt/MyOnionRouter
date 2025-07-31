#include "protocol/relay_data_structs.h"
#include "protocol/tor_structs.h"
#include "session.h"

bool tor_send_create_msg(client_session_t* session, msg_tor_buffer_t* buffer, const identity_key_t* id_key);
bool tor_send_extend_msg(client_session_t* session, msg_tor_buffer_t* buffer, const identity_key_t* id_key, const relay_descriptor_t* relay_descriptor);
bool tor_send_begin_msg(client_session_t* session, msg_tor_buffer_t* buffer, uint32_t stream_id, sock_addr_t* addr);
bool tor_send_data_msg (client_session_t* session, msg_tor_buffer_t* buffer, uint32_t stream_id, uint8_t* data, uint16_t length);
bool tor_send_end_msg  (client_session_t* session, msg_tor_buffer_t* buffer, uint32_t stream_id);
bool tor_send_ping_msg (client_session_t* session, msg_tor_buffer_t* buffer);
bool tor_send_destroy_msg (client_session_t* session, msg_tor_buffer_t* buffer);

void tor_parse_end_msg      (msg_tor_buffer_t* buffer, uint32_t* stream_id, tor_relay_end_reasons_t* reason);
