#include "relay_manager.h"
#include <handlers.h>

#include <encryptions/encryptions.h>
#include <protocol/server_net_structs.h>
#include <socket_context.h>
#include <net_messages.h>

#include <stdio.h>
#include <unistd.h>

typedef struct client_data
{
    uint32_t relay_start;
} client_data_t;

static bool handle_get_relay_map(int sock_fd, client_data_t* client_data, key_data_t* key, msg_buffer_t* buffer)
{
    server_relay_list_t relay_list;

    relay_list.relay_amount = get_relay_batch(relay_list.relays, client_data->relay_start, SERVER_RELAYS_MAP_AMOUNT);

    client_data->relay_start += relay_list.relay_amount; // Not correct calculation, will work now for simplcity
 
    return send_enc_client_relay_map(sock_fd, key, buffer, &relay_list, server_response_success);
}

static bool handle_exit(int sock_fd, client_data_t* client_data, key_data_t* key, msg_buffer_t* buffer)
{    
    (void)client_data; // not used

    return send_enc_relay_exit_response(sock_fd, key, buffer, server_response_success);
}


static bool handle_message(int sock_fd, client_data_t* client_data, key_data_t* key, msg_buffer_t* buffer, bool* running)
{
    if (recv_enc_server_msg(sock_fd, buffer, key) == false)
    {
        return false;   
    }

    bool return_value = false;

    server_client_request_t* client_request = (server_client_request_t*)buffer;
    switch (client_request->command)
    {
    case CLIENT_COMMAND_GET_RELAY_MAP:
        return_value = handle_get_relay_map(sock_fd, client_data, key, buffer);
        break;
    
    case CLIENT_COMMAND_EXIT:
        return_value = handle_exit(sock_fd, client_data, key, buffer);
        *running = false;
        break;
    
    default:
        fprintf(stderr, "Invalid client message: %d\n", client_request->command);
        return_value = false;
    } 

    return return_value;
}


int process_client(int sock_fd, msg_buffer_t* buffer)
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

    bool running = true;
    
    client_data_t client_data = {
        .relay_start=0
    };

    while (running)
    {
        if (handle_message(sock_fd, &client_data, key, buffer, &running) == false)
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