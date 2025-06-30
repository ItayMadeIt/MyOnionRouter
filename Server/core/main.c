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
        
    // correct: ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad
    for (int i = 0; i < 32; i++) printf("%02x ", hash[i]);


    return 0;
}