#include "relay_manager.h"

#include <handlers.h>
#include <protocol/server_net_structs.h>
#include <socket_context.h>
#include <net_messages.h>
#include <encryptions/encryptions.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static bool handle_signup(int sock_fd, relay_data_t** relay_data, key_data_t* key, msg_server_buffer_t* buffer)
{
    if (*relay_data != NULL)
    {
        return false;
    }

    server_relay_request_signup_t request = *(server_relay_request_signup_t*)buffer;

    *relay_data = gen_relay();
    get_other_public_key(key, (*relay_data)->descriptor.public_key);
    memcpy(&(*relay_data)->descriptor.sock_addr, &request.relay_addr, sizeof(sock_addr_t));

    uint32_t relay_id = (*relay_data)->relay_id;

    return send_enc_relay_signup_response(sock_fd, key, buffer, server_response_success, relay_id);
}

static bool handle_signout(int sock_fd, relay_data_t** relay_data, key_data_t* key, msg_server_buffer_t* buffer)
{
    server_relay_request_signout_t* request = (server_relay_request_signout_t*)buffer;
    
    // Assuming if sign'ed up this session, it knows the id
    if (*relay_data != NULL)
    {
        remove_relay((*relay_data)->relay_id);

        *relay_data = NULL;

        return send_enc_relay_signout_response(sock_fd, key, buffer, server_response_success);
    }

    *relay_data = get_relay(request->assigned_id);

    if (*relay_data == NULL)
    {
        return send_enc_relay_signout_response(sock_fd, key, buffer, server_response_base_error);
    }

    uint8_t true_public_key[ASYMMETRIC_KEY_BYTES];
    get_other_public_key(key, true_public_key);

    if (memcmp((*relay_data)->descriptor.public_key, true_public_key, ASYMMETRIC_KEY_BYTES) != 0)
    {
        print_asymmertic((*relay_data)->descriptor.public_key);
        print_asymmertic(true_public_key);
        return send_enc_relay_signout_response(sock_fd, key, buffer, server_response_base_error);
    }

    remove_relay((*relay_data)->relay_id);
    *relay_data = NULL;

    return send_enc_relay_signout_response(sock_fd, key, buffer, server_response_success);
}

static bool handle_exit(int sock_fd, relay_data_t** relay_data, key_data_t* key, msg_server_buffer_t* buffer)
{
    // No need to access relay data anymore
    *relay_data = NULL;

    return send_enc_relay_exit_response(sock_fd, key, buffer, server_response_success);
}

static bool handle_message(int sock_fd, relay_data_t** relay_data, key_data_t* key, msg_server_buffer_t* buffer, bool* running)
{
    if (recv_enc_server_msg(sock_fd, buffer, key) == false)
    {
        return false;   
    }

    bool return_value = false;

    server_relay_request_t* relay_request = (server_relay_request_t*)buffer;
    switch (relay_request->command)
    {
    case RELAY_COMMAND_SIGNUP:
        return_value = handle_signup(sock_fd, relay_data, key, buffer);
        break;
    
    case RELAY_COMMAND_SIGNOUT:
        return_value = handle_signout(sock_fd, relay_data, key, buffer);
        break;

    case RELAY_COMMAND_EXIT:
        return_value = handle_exit(sock_fd, relay_data, key, buffer);
        *running = false;
        break;
    
    default:
        fprintf(stderr, "Invalid relay message: %d\n", relay_request->command);
        return_value = false;
    } 

    return return_value;
}

int process_relay(int sock_fd, msg_server_buffer_t* buffer)
{
    if (buffer == NULL)
    {
        return handler_error;
    }
    
    key_data_t* key = get_sock_key(sock_fd);
    if (key == NULL)
    {
        return handler_error;
    }

    relay_data_t* relay = NULL;

    bool running = true;
    
    while (running)
    {
        if (handle_message(sock_fd, &relay, key, buffer, &running) == false)
        {
            free_socket(sock_fd, NULL);
            close(sock_fd);

            return handler_error;
        }
    }

    free_socket(sock_fd, NULL);

    close(sock_fd);

    return handler_success;
}