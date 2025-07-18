#include <server_register.h>

#include <relay.h>
#include <protocol/server_net_structs.h>
#include <stdio.h>
#include <string.h>
#include <utils/sock_utils.h>
#include <net_messages.h>

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

static void convert_sock_addr(sock_addr_t* result, const struct sockaddr_storage* storage)
{
    if (storage->ss_family == AF_INET) 
    {
        const struct sockaddr_in* addr4 = (const struct sockaddr_in*)storage;
        result->family = AF_INET;
        result->protocol = IPPROTO_TCP;
        memcpy(result->addr, &addr4->sin_addr, sizeof(struct in_addr));
        result->port = addr4->sin_port;
    }
    else if (storage->ss_family == AF_INET6) 
    {
        const struct sockaddr_in6* addr6 = (const struct sockaddr_in6*)storage;
        result->family = AF_INET6;
        result->protocol = IPPROTO_TCP;
        memcpy(result->addr, &addr6->sin6_addr, sizeof(struct in6_addr));
        result->port = addr6->sin6_port;
    }
    else 
    {
        memset(result, 0, sizeof(sock_addr_t));
    }
}

static relay_code_t handle_handshake(int sock_fd, msg_server_buffer_t* buffer)
{
    if (send_server_handshake_req(sock_fd, buffer) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    server_handshake_response_t handshake_response;
    if (recv_server_handshake_res(sock_fd, buffer, &handshake_response) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    // Setup identity key
    if (relay_vars.is_key_init == false)
    {
        set_globals(handshake_response.g, handshake_response.p);

        init_id_key(&relay_vars.key);

        // Setup a server key
        init_key(&relay_vars.server_key, &relay_vars.key);
        derive_symmetric_key_from_public(&relay_vars.server_key, handshake_response.server_pubkey);

        relay_vars.is_key_init = true;
    }

    uint8_t public_key[ASYMMETRIC_KEY_BYTES];
    get_public_identity_key(&relay_vars.key, public_key);

    if (send_server_handshake_key(sock_fd, buffer, public_key) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    server_handshake_confirmation_t confirmation;
    if (recv_server_handshake_confirmation(sock_fd, buffer, &relay_vars.server_key, &confirmation) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    if (memcmp(SERVER_HANDSHAKE_V1_MAGIC, confirmation.magic, SERVER_HANDSHAKE_V1_MAGIC_LEN) != 0)
    {
        fprintf(stderr, "MAGIC V1 value didn't match: %s\n", confirmation.magic);

        close(sock_fd);

        return relay_error;
    }

    return relay_success;
}

static relay_code_t handle_exit(int sock_fd, msg_server_buffer_t* buffer)
{
    if (send_server_exit_req(sock_fd, buffer, &relay_vars.server_key) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    if (recv_server_exit_res(sock_fd, buffer, &relay_vars.server_key) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    return relay_success;
}

static relay_code_t handle_signup(int sock_fd, msg_server_buffer_t* buffer, sock_addr_t* sock_addr, uint32_t* id)
{
    if (send_server_signup_req(sock_fd, buffer, &relay_vars.server_key, sock_addr) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    if (recv_server_signup_res(sock_fd, buffer, &relay_vars.server_key, id) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    return relay_success;
}

static relay_code_t handle_signout(int sock_fd, msg_server_buffer_t* buffer, uint32_t id)
{
    if (send_server_signout_req(sock_fd, buffer, &relay_vars.server_key, id) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    if (recv_server_signout_res(sock_fd, buffer, &relay_vars.server_key) == false)
    {
        close(sock_fd);
        return relay_error;
    }

    return relay_success;
}

relay_code_t signup_server(struct sockaddr_storage* sock_storage)
{
    msg_server_buffer_t buffer;

    if (relay_vars.register_id != INVALID_RELAY_ID)
    {
        printf("Invalid id\n");
        return relay_error;
    }

    int sock_fd = connect_server(relay_vars.config->server_cfg);

    if (sock_fd == -1)
    {
        printf("Failed connection\n");
        return relay_error;
    }

    relay_code_t response = handle_handshake(sock_fd, &buffer);
    if (response != relay_success)
    {
        printf("Failed handshake\n");
        return response;
    }

    sock_addr_t sock_addr;
    convert_sock_addr(&sock_addr, sock_storage);
        
    response = handle_signup(sock_fd, &buffer, &sock_addr, &relay_vars.register_id);
    if (response != relay_success)
    {
        printf("Failed signup\n");
        return response;
    }

    printf("ID: %d\n", relay_vars.register_id);

    response = handle_exit(sock_fd, &buffer);    

    close(sock_fd);

    return relay_success;
}

relay_code_t signout_server()
{
    msg_server_buffer_t buffer;

    if (relay_vars.register_id == INVALID_RELAY_ID)
    {
        return relay_error;
    }

    int sock_fd = connect_server(relay_vars.config->server_cfg);

    if (sock_fd == -1)
    {
        return relay_error;
    }

    relay_code_t response = handle_handshake(sock_fd, &buffer);
    if (response != relay_success)
    {
        return response;
    }

    response = handle_signout(sock_fd, &buffer, relay_vars.register_id);
    if (response != relay_success)
    {
        return response;
    }

    printf("ID: %d\n", relay_vars.register_id);

    relay_vars.register_id = INVALID_RELAY_ID;

    response = handle_exit(sock_fd, &buffer);    

    close(sock_fd);

    return relay_success;
}