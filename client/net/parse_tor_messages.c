#include "protocol/tor_structs.h"
#include "tor_net_messages.h"

void tor_parse_end_msg(msg_tor_buffer_t* buffer, uint32_t* stream_id, tor_relay_end_reasons_t* reason)
{
    msg_tor_relay_end_t* end = (msg_tor_relay_end_t*)buffer; 
    *stream_id = end->stream_id;
    *reason = end->reason;
}