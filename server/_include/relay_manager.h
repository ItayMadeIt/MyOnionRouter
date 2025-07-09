#ifndef __RELAY_MANAGER_H__
#define __RELAY_MANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include <protocol/relay_data_structs.h>

typedef struct relay_data_item
{
    bool exists;
    
    relay_data_t data;

} relay_data_item_t;

// Relay data by id, if doesn't exist it's NULL
relay_data_t* get_relay(uint32_t id);

// Creates a relay and assigns an id
relay_data_t* gen_relay();

// Removes relay
bool remove_relay(uint32_t id);

void setup_relay_manager();
#endif // __RELAY_MANAGER_H__