#ifndef __SESSION_H__
#define __SESSION_H__

#include "client.h"
#include "client_config.h"
#include <encryptions/encryptions.h>

typedef struct client_session {
    
    key_data_t onion_keys[MAX_RELAYS];
    key_data_t tls_key; 

    int sock_fd;

    const client_config_metadata_t* config;

} client_session_t;

void init_session(client_session_t* session, int sock_fd);
bool process_session(client_session_t* session);
void free_session(client_session_t* session);

#endif // __SESSION_H__