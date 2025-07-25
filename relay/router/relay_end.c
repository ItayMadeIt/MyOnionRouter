#include "protocol/tor_structs.h"
#include "session.h"
#include "relay.h"
#include <stdio.h>

relay_code_t process_end_relay_session(relay_session_t* session, msg_tor_buffer_t* buffer)
{
    (void)session; // not using it for now

    msg_tor_t* msg = (msg_tor_t*)buffer;
    printf("Does something with session, cmd: %d\n\n", msg->cmd);

    return relay_success;
}