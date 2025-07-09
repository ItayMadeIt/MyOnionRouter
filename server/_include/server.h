#ifndef __SERVER_H__
#define __SERVER_H__

typedef enum server_code
{
    server_success = 0,
    server_error   = 1,
} server_code_t;

server_code_t run_server(const char* filepath);
#endif // __SERVER_H__