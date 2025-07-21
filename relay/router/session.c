#include "session.h"

#include "relay.h"
#include <protocol/tor_structs.h>
#include <net_messages.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void free_session(relay_session_t *session_vars)
{
    free_key(&session_vars->session_key);
    free_key(&session_vars->tls_last_key);
    free_key(&session_vars->tls_next_key);

    if (session_vars->last_fd < 0)
    {
        close(session_vars->last_fd);
    }
    
    if (session_vars->next_fd < 0)
    {
        close(session_vars->next_fd);
    }
}

bool handle_create_message(relay_session_t* session_var, msg_tor_buffer_t* buffer)
{
    // Get tor CREATE (with client key)
    recv_tor_buffer(session_var->last_fd, buffer, &session_var->tls_last_key, NULL);

    msg_tor_t* recv_msg = (msg_tor_t*)&buffer;
    // cmd has to be TOR_CREATE
    if (recv_msg->cmd != TOR_CREATE)
    {
        fprintf(stderr, "TOR_CREATE wasn't first message");
        printf("cmd: %d, circuit id: %d\n",recv_msg->cmd, recv_msg->circID);
        return false;
    }
    
    tor_create_data_t* create_data = (tor_create_data_t*)recv_msg->data;
    derive_symmetric_key_from_public(&session_var->session_key, create_data->public_client_key);

    // Respond with CREATED, no real data
    msg_tor_t* send_msg = (msg_tor_t*)&buffer;
    send_msg->cmd = TOR_CREATED;
    memset(send_msg->data, 0, ASYMMETRIC_KEY_BYTES);

    send_tor_buffer(session_var->last_fd, buffer, &session_var->tls_last_key, NULL);

    return true;
}

bool process_session(relay_session_t* session)
{
    msg_tor_buffer_t buffer;

    if (handle_create_message(session, &buffer) == false)
    {
        free_session(session);
        return false;
    }

    return true;
}