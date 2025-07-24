#include "relay.h"

void init_session(relay_session_t* session, int sock_fd);
relay_code_t process_session(relay_session_t* session);
void free_session(relay_session_t* session_vars);