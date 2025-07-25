#include "relay.h"

#include <protocol/tor_structs.h>

void init_relay_session(relay_session_t* session, int sock_fd);
relay_code_t process_relay_session(relay_session_t* session);
void free_relay_session(relay_session_t* session_vars);

relay_code_t process_middle_relay_session(relay_session_t* session, msg_tor_buffer_t* buffer);
relay_code_t process_end_relay_session(relay_session_t* session, msg_tor_buffer_t* buffer);
