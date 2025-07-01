#include <stdio.h>
#include <server.h>
#include <aes.h>
#include <sha256.h>

#include <inttypes.h>


#define ARGS 2

void str_to_block128(aes_block128_t* block, const uint8_t input[16])
{
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            block->bytes[col][row] = input[col * 4 + row];
        }
    }
}

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

    uint8_t arr[64] = "abc";
    hash_prepare_data_sha256(arr, 3, 64);

    uint8_t hash[32];
    hash_sha256(arr, 64, hash);
        
    // correct: ba 78 16 bf 8f 01 cf ea 41 41 40 de 5d ae 22 23 b0 03 61 a3 96 17 7a 9c b4 10 ff 61 f2 00 15 ad
    for (int i = 0; i < 32; i++) printf("%02x ", hash[i]);


    return 0;
}