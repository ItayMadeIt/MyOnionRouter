#include <stdint.h>

#include "protocol/server_net_structs.h"

typedef enum handler_response {
    handler_success = 0,
    handler_error = 1,
} handler_response_t;

int process_relay (int sock_fd, msg_buffer_t* buffer);
int process_client(int sock_fd, msg_buffer_t* buffer);
