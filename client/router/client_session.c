#include <session.h>

#include <utils/debug.h>

#include "client.h"
#include "net_messages.h"
#include "protocol/tor_structs.h"

#include <handle_tls.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void init_session(client_session_t* session, int sock_fd, const circuit_relay_list_t* relay_list)
{
    session->sock_fd = sock_fd;
    session->config = client_vars.config;
    for (uint8_t i = 0; i < relay_list->relay_amount; i++)
    {
        init_key(&session->onion_keys[i], &client_vars.id_key);
        derive_symmetric_key_from_public(&session->onion_keys[i], relay_list->relays[i].public_key);
    }
    init_key(&session->tls_key, &client_vars.id_key);
}

static bool handle_create_message(client_session_t* session, msg_tor_buffer_t* buffer)
{
    // Send CREATE msg
    msg_tor_create_t* create_tor = (msg_tor_create_t*)buffer;
    create_tor->circID = 0;
    create_tor->cmd = TOR_CREATE;
    get_public_identity_key(&client_vars.id_key, create_tor->data);

    if (send_tor_buffer(session->sock_fd, buffer, &session->tls_key, NULL, 0) == false)
    {
        return false;
    }

    // Send CREATE msg
    msg_tor_created_t* response = (msg_tor_created_t*)buffer;
    if (recv_tor_buffer(session->sock_fd, buffer, &session->tls_key, NULL, 0) == false)
    {
        return false;
    }
    printf("a\n");
    
    if (response->cmd != TOR_CREATED)
    {
        free_session(session);
        return client_error;
    } 
    
    return client_success;
}

client_code_t process_session(client_session_t* session)
{
    client_code_t return_value = client_success;

    msg_tor_buffer_t buffer = {0};
    return_value = handle_create_message(session, &buffer);
    if (return_value != client_success)
    {
        free_session(session);
        return return_value;
    }

    session->cur_relays = 1;

    while (true)
    {
        recv_tor_buffer(
            session->sock_fd, 
            &buffer, 
            &session->tls_key,
            session->onion_keys,
            session->cur_relays
        );

    }    

    return client_success;
}

void free_session(client_session_t* session)
{
    if (session->sock_fd > 0)
    {
        close(session->sock_fd);
        session->sock_fd = -1;
    }
    free_key(&session->tls_key);
    for (uint8_t i = 0; i < session->config->relays; i++)
    {
        free_key(&session->onion_keys[i]);
    }
}