#ifndef __SESSION_H__
#define __SESSION_H__

#include "client.h"
#include "client_config.h"
#include "stream_hashmap.h"
#include <encryptions/encryptions.h>

typedef struct client_session {
    
    key_data_t onion_keys[MAX_RELAYS];
    key_data_t tls_key; 

    int sock_fd;

    const circuit_relay_list_t* relays;
    uint8_t cur_relays; 
    const client_config_metadata_t* config;

    stream_hashmap_t* hashmap;

} client_session_t;

void init_session(client_session_t* session, int sock_fd, const circuit_relay_list_t* relay_list);
client_code_t process_client_session(client_session_t* session);
void free_session(client_session_t* session);

#endif // __SESSION_H__