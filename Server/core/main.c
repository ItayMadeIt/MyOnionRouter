#include <stdio.h>
#include <server.h>

#define ARGS 2

int main(int argc, char *argv[])
{
    if (argc != ARGS)
    {
        fprintf(stderr, "Use: server <config_file>\n");
        return -1;
    }
    
    server_code_t code = run_server(argv[1]);
    
    if (code != server_success)
    {
        fprintf(stderr, "Server failed");
        return 1;
    }

    return 0;
}