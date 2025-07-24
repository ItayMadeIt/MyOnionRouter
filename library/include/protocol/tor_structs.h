#ifndef __TOR_STRUCTS_H__
#define __TOR_STRUCTS_H__

#include <encryptions/encryptions.h>
#include <stdint.h>
#include <protocol/relay_data_structs.h>

#define RELAY_MSG_SIZE 512

typedef struct msg_tor_buffer {
    uint8_t data[RELAY_MSG_SIZE];
} __attribute__((packed)) msg_tor_buffer_t;


typedef enum tor_commands
{
    TOR_PADDING  = 0,
    TOR_CREATE   = 1,
    TOR_CREATED  = 2,
    TOR_RELAY    = 3,
    TOR_DESTORY  = 4,
} tor_commands_t;

typedef enum tor_relay_commands
{
    RELAY_BEGIN     = 1,
    RELAY_DATA      = 2,
    RELAY_END       = 3,
    RELAY_CONNECTED = 4,
    RELAY_SENDME    = 5, // Specifying window
    RELAY_EXTEND    = 6, // Extends, adds 1 circuit
    RELAY_EXTENDED  = 7, // Extended - extend responsse

} tor_relay_commands_t;

typedef struct msg_tor {
    uint16_t circID;
    uint8_t cmd; 
    uint8_t data[RELAY_MSG_SIZE - sizeof(uint16_t) - sizeof(uint8_t)];
} __attribute__((packed)) msg_tor_t;


typedef struct msg_tor_create {
    uint16_t circID;
    uint8_t cmd; 
    uint8_t public_client_key[ASYMMETRIC_KEY_BYTES];
    uint8_t data[RELAY_MSG_SIZE - sizeof(uint16_t) - sizeof(uint8_t) - ASYMMETRIC_KEY_BYTES];
} __attribute__((packed)) msg_tor_create_t;

typedef struct msg_tor_created {
    uint16_t circID;
    uint8_t cmd; 
    uint8_t data[RELAY_MSG_SIZE - sizeof(uint16_t) - sizeof(uint8_t)];
} __attribute__((packed)) msg_tor_created_t;


#define DIGEST_LEN 6
typedef struct msg_tor_relay {
    uint16_t circID;
    uint8_t relay; // CMD = RELAY  
    
    uint16_t streamID;
    uint8_t digest[DIGEST_LEN];
    uint16_t length;
    uint8_t cmd;

    uint8_t data[RELAY_MSG_SIZE - sizeof(uint16_t) - sizeof(uint8_t)
                - sizeof(uint16_t) - sizeof(uint8_t)*DIGEST_LEN - sizeof(uint16_t) - sizeof(uint8_t)];
    
} __attribute__((packed)) msg_tor_relay_t;


typedef struct msg_tor_relay_extend {
    uint16_t circID;
    uint8_t relay; // CMD = RELAY  
    
    uint16_t streamID;
    uint8_t digest[DIGEST_LEN];
    uint16_t length;
    uint8_t cmd;

    sock_addr_t relay_addr;
    uint8_t client_public_key[ASYMMETRIC_KEY_BYTES];

    uint8_t data[RELAY_MSG_SIZE - sizeof(uint16_t) - sizeof(uint8_t)
                - sizeof(uint16_t) - sizeof(uint8_t)*DIGEST_LEN - sizeof(uint16_t) - sizeof(uint8_t)
                - sizeof(sock_addr_t) - sizeof(uint8_t) * ASYMMETRIC_KEY_BYTES];
    
} __attribute__((packed)) msg_tor_relay_extend_t;


typedef struct msg_tor_relay_extended {
    uint16_t circID;
    uint8_t relay; // CMD = RELAY  
    
    uint16_t streamID;
    uint8_t digest[DIGEST_LEN];
    uint16_t length;
    uint8_t cmd;

    uint8_t data[RELAY_MSG_SIZE - sizeof(uint16_t) - sizeof(uint8_t)
                - sizeof(uint16_t) - sizeof(uint8_t)*DIGEST_LEN - sizeof(uint16_t) - sizeof(uint8_t)];
    
} __attribute__((packed)) msg_tor_relay_extended_t;

typedef struct tls_key_buffer {
    uint8_t public_key[ASYMMETRIC_KEY_BYTES];
} __attribute__((packed)) tls_key_buffer_t;
#endif // __TOR_STRUCTS_H__