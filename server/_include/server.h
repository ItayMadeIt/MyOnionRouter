#ifndef __SERVER_H__
#define __SERVER_H__

#include <encryptions/encryptions.h>
#include <sys/socket.h>

typedef enum server_code
{
    server_success = 0,
    server_error   = 1,
} server_code_t;

extern global_data_t encryption_globals;
extern identity_key_t server_id_key;

server_code_t run_server(const char* filepath);
#endif // __SERVER_H__