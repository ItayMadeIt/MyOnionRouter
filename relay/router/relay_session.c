#include "session.h"

#include "relay.h"
#include <protocol/tor_structs.h>
#include <net_messages.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void init_relay_session(relay_session_t* session, int sock_fd)
{
    session->last_fd = sock_fd;
    session->next_fd = -1;

    init_key(&session->onion_key, &relay_vars.key);
    init_key(&session->tls_last_key, &relay_vars.key);
    init_key(&session->tls_next_key, &relay_vars.key);
}
void free_relay_session(relay_session_t *session)
{
    free_key(&session->onion_key);
    free_key(&session->tls_last_key);
    free_key(&session->tls_next_key);

    if (session->last_fd >= 0)
    {
        close(session->last_fd);
    }
    
    if (session->next_fd >= 0)
    {
        close(session->next_fd);
    }
}

relay_code_t handle_create_message(relay_session_t* session_var, msg_tor_buffer_t* buffer)
{
    // Get tor CREATE (with client key)
    if (recv_tor_buffer(session_var->last_fd, buffer, &session_var->tls_last_key, NULL, true) == false)
    {
        return relay_error;
    }
    
    msg_tor_t* recv_msg = (msg_tor_t*)buffer;

    // cmd has to be TOR_CREATE
    if (recv_msg->cmd != TOR_CREATE)
    {
        fprintf(stderr, "TOR_CREATE wasn't first message");
        return relay_error;
    }

    msg_tor_create_t* create_msg = (msg_tor_create_t*)recv_msg;
    uint8_t relay_public_key[ASYMMETRIC_KEY_BYTES];
    get_public_identity_key(session_var->onion_key.identity, relay_public_key);
    derive_symmetric_key_from_public(&session_var->onion_key, create_msg->public_client_key);

    // Respond with CREATED, no real data
    msg_tor_created_t* created_msg = (msg_tor_created_t*)buffer;
    created_msg->cmd = TOR_CREATED;
    memset(created_msg->data, 0, ASYMMETRIC_KEY_BYTES);   

    send_tor_buffer(session_var->last_fd, buffer, &session_var->tls_last_key, NULL, true);
    
    return relay_success;
}

relay_code_t process_relay_session(relay_session_t* session)
{
    msg_tor_buffer_t buffer;

    relay_code_t return_value = relay_success;

    return_value = handle_create_message(session, &buffer);
    if (return_value != relay_success)
    {
        free_relay_session(session);
        return return_value;
    }
    printf("Create message accepted\n");

    if (recv_tor_buffer(session->last_fd, &buffer, &session->tls_last_key, &session->onion_key, true) == false)
    {
        printf("Failed to recv first tor buffer\n");
        free_relay_session(session);
        return return_value;
    }

    msg_tor_relay_t* msg = (msg_tor_relay_t*)&buffer;
    if (msg->relay != TOR_RELAY)
    {
        printf("First tor message isn't relay msg %d\n", msg->relay);
        free_relay_session(session);
        return relay_unexpected_msg;
    }

    printf("msg relay cmd: %d\n", msg->cmd);
    if (msg->cmd == RELAY_EXTEND)
    {
        printf("Process middle relay\n");
        return process_middle_relay_session(session, &buffer);
    }
    else
    {
        printf("Process last extend\n");
        return process_end_relay_session(session, &buffer);
    }
}