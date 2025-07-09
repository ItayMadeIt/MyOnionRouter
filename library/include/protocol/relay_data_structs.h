#ifndef __RELAY_DATA_STRUCTS_H__
#define __RELAY_DATA_STRUCTS_H__

#include <stdint.h>
#include <stdbool.h>
#include <encryptions/encryptions.h>

#define MAX(a, b) (a > b ? a : b)
#define IP6_SIZE 16
#define IP4_SIZE 4

typedef struct relay_sock_addr {
    
    uint8_t family;   // AF_INET, AF_INET6
    uint8_t protocol; // currently only support TCP

    uint8_t addr[MAX(IP4_SIZE, IP6_SIZE)]; // addr, ipv6 or ipv4
    uint16_t port;

} __attribute__((packed)) relay_sock_addr_t;

typedef struct relay_descriptor {

    relay_sock_addr_t sock_addr;

    // Known public key of relay
    uint8_t public_key[ASYMMETRIC_KEY_BYTES];

    // Unique id for each relay
    uint32_t relay_id;
    
} __attribute__((packed)) relay_descriptor_t;

typedef struct relay_data {
    // Relay specific data
    relay_descriptor_t descriptor;

    // Common key
    uint8_t common_key[SYMMETRIC_KEY_BYTES];

} __attribute__((packed)) relay_data_t;

#endif // __RELAY_DATA_STRUCTS_H__