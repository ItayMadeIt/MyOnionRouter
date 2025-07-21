
#include "net_messages.h"
#include "protocol/tor_structs.h"
#include <session.h>

#include <handle_tls.h>
#include <stdio.h>
#include <unistd.h>

void init_session(client_session_t* session, int sock_fd)
{
    session->sock_fd = sock_fd;
    session->config = client_vars.config;
    for (uint8_t i = 0; i < session->config->relays; i++)
    {
        init_key(&session->onion_keys[i], &client_vars.id_key);
    }
    init_key(&session->tls_key, &client_vars.id_key);
}

bool process_session(client_session_t* session)
{
    if (handle_tls_sender(session->sock_fd, &session->tls_key) == false)
    {
        session->sock_fd = -1;
        free_session(session);
        return false;
    }

    // Sample CREATE and recv CREATED
    msg_tor_t create_msg;
    create_msg.circID=0;
    create_msg.cmd=TOR_CREATE;
    
    tor_create_data_t* create_data = (tor_create_data_t*)create_msg.data;
    get_public_identity_key(&client_vars.id_key, create_data->public_client_key);

    send_tor_buffer(session->sock_fd, (msg_tor_buffer_t*)&create_msg, &session->tls_key, NULL);

    msg_tor_t response;
    recv_tor_buffer(session->sock_fd, (msg_tor_buffer_t*)&response, &session->tls_key, NULL);
    if (response.cmd != TOR_CREATED)
    {
        printf("Response failed");
        
        free_session(session);
        return false;
    }

    return true;
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