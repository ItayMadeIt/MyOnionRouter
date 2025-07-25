#include <netinet/in.h>
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
    msg_tor_create_t* create_msg = (msg_tor_create_t*)buffer;
    create_msg->circID = 0;
    create_msg->cmd = TOR_CREATE;
    get_public_identity_key(&client_vars.id_key, create_msg->public_client_key);

    send_tor_buffer(session->sock_fd, buffer, &session->tls_key, NULL, 0);

    // Send CREATE msg
    msg_tor_created_t* response = (msg_tor_created_t*)buffer;
    recv_tor_buffer(session->sock_fd, buffer, &session->tls_key, NULL, 0);
    
    if (response->cmd != TOR_CREATED)
    {
        free_session(session);
        return client_error;
    } 
    
    return client_success;
}

client_code_t create_relay_circuit(client_session_t* session, msg_tor_buffer_t* buffer)
{
    while (session->cur_relays < session->relays->relay_amount)
    {
        printf("cur relay: %d\n", session->cur_relays);

        msg_tor_relay_extend_t* msg_extend = (msg_tor_relay_extend_t*)buffer;
        msg_extend->relay = TOR_RELAY;

        msg_extend->cmd = RELAY_EXTEND;
        msg_extend->circID = 0;
        memset(&msg_extend->digest, 0, DIGEST_LEN);
        
        get_public_identity_key(session->tls_key.identity, msg_extend->client_public_key);
        
        memcpy(
            &msg_extend->relay_addr, 
            &session->relays->relays[session->cur_relays], 
            sizeof(sock_addr_t)
        );

        printf("Send tor extend, port %d\n", ntohs(session->relays->relays[session->cur_relays].sock_addr.port));
        // session->onion_keys
        if (!send_tor_buffer(
            session->sock_fd, buffer,  &session->tls_key, session->onion_keys, session->cur_relays))
        {
            close(session->sock_fd);
            return client_error;
        }

        printf("Recv tor extended\n");
        if (!recv_tor_buffer(
            session->sock_fd, buffer, &session->tls_key, session->onion_keys, session->cur_relays))
        {
            close(session->sock_fd);
            return client_error;            
        }
        printf("Recv data\n");

        msg_tor_relay_extended_t* extended = (msg_tor_relay_extended_t*)buffer;
        printf("Recv cmd: %d\n", extended->relay);
        if (extended->cmd != RELAY_EXTENDED)
        {
            close(session->sock_fd);
            return client_error;
        }

        printf("next relay: %d\n", session->cur_relays);
        session->cur_relays++;
    }

    return client_success;
}

client_code_t process_client_session(client_session_t* session)
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
    return_value = create_relay_circuit(session, &buffer);
    if (return_value != client_success)
    {
        session->sock_fd = -1;
        free_session(session);
        return return_value;
    }

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