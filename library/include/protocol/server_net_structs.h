#ifndef __SERVER_NET_STRUCTS_H__
#define __SERVER_NET_STRUCTS_H__

#include <encryptions/encryptions.h>
#include <stdint.h>
#include "relay_data_structs.h"

#define SERVER_MSG_SIZE 4096
#define SERVER_HANDSHAKE_VERSION 1
#define SERVER_HANDSHAKE_V1_MAGIC "M0R1"
#define SERVER_HANDSHAKE_V1_MAGIC_LEN (sizeof(SERVER_HANDSHAKE_V1_MAGIC) - 1)

#define SERVER_RELAYS_MAP_AMOUNT ((SERVER_MSG_SIZE - sizeof(uint16_t)*2)/ sizeof(relay_descriptor_t))

typedef struct msg_buffer {
    uint8_t data[SERVER_MSG_SIZE];
} msg_buffer_t;

typedef struct server_relay_list {
    uint16_t relay_amount;
    relay_descriptor_t relays[SERVER_RELAYS_MAP_AMOUNT];

} __attribute__((packed)) server_relay_list_t;

// User types
typedef enum prot_server_user_types
{
    prot_user_type_invalid = 0,
    prot_user_type_client  = 1,
    prot_user_type_relay   = 2,
} prot_server_user_types_t;


// Version one, protocol
typedef struct server_handshake_request
{
    uint16_t version;       // Protocol version
    uint8_t user_type;      // server_user_types_t (1 byte)
    uint8_t flags;          // Bit flags (unused)
    
} __attribute__((packed)) server_handshake_request_t;


typedef struct server_handshake_response
{
    uint64_t g;                                  // Generator
    uint8_t p[ASYMMETRIC_KEY_BYTES];             // Prime modulus
    uint8_t server_pubkey[ASYMMETRIC_KEY_BYTES]; // g^x mod p

} __attribute__((packed)) server_handshake_response_t;

typedef struct server_handshake_client_key
{
    // g^y mod p (client’s or relay’s DH public key)
    uint8_t client_pubkey[ASYMMETRIC_KEY_BYTES]; 

} __attribute__((packed)) server_handshake_client_key_t;

typedef struct server_handshake_confirmation
{
    // Magic (changes each version, for 1: "M0R1"-SERVER_HANDSHAKE_V1_MAGIC)
    uint8_t magic[SERVER_HANDSHAKE_V1_MAGIC_LEN];
    
    // Only meaningful for relays (for clients may be used, for now = 0)
    uint32_t assigned_id;

    // Time it was sent
    uint64_t timestamp; 

} __attribute__((packed)) server_handshake_confirmation_t;

const server_handshake_confirmation_t base_confirmation = {SERVER_HANDSHAKE_V1_MAGIC, 0, 0};

typedef enum relay_command_t {
    RELAY_COMMAND_LOGIN   = 1,
    RELAY_COMMAND_SIGNOUT = 2
} relay_command_t;

typedef enum client_command_t {
    CLIENT_COMMAND_GET_RELAY_MAP = 1
} client_command_t;


typedef struct server_relay_request
{
    uint16_t command; // Only 2 commands now, relay login, relay signout
} __attribute__((packed)) server_relay_request_t;

typedef struct server_relay_request_login
{
    uint16_t command; // relay login

    relay_sock_addr_t relay_addr;

} __attribute__((packed)) server_relay_request_login_t;

typedef struct server_relay_request_signout
{
    uint16_t command; // relay signout

    uint32_t assigned_id; // relay id (from handshake confirmation)

} __attribute__((packed)) server_relay_request_signout_t;

typedef struct server_client_request
{
    uint16_t command; // Only 1 command now, client request relay map
} __attribute__((packed)) server_client_request_t;

typedef struct server_client_request_map
{
    uint16_t command; // Request relay map
    server_relay_list_t relays;
} __attribute__((packed)) server_client_request_map_t;



#endif // __SERVER_NET_STRUCTS_H__