#include <aes.h>

#include <stdio.h>

void print_block128(aes_block128_t* block)
{
    for (int i = 0; i < 4; ++i) 
    {
        for (int j = 0; j < 4; ++j) 
        {
            printf("%02X ", block->bytes[i][j]);
        }
    }
    printf("\n ");
    for (int i = 0; i < 4; ++i) 
    {
        for (int j = 0; j < 4; ++j) 
        {
            printf("%02X ", block->bytes[j][i]);
        }
        printf("\n ");
    }
    printf("\n");
}

void aes_encrypt128(const aes_block128_t* data   , const aes_block128_t* key, aes_block128_t* result)
{
    // result = data 
    cpy_block128(result, data);

    // Generate keys
    aes_keys_t keys;
    aes_generate_keys128(key, keys);

    // Apply first round key
    xor_block128(result, &keys[0]);

    print_block128(result);

    // Rounds 1 to AES_BLOCK128_ROUNDS-1
    for (uint16_t i = 1; i <= AES_BLOCK128_ROUNDS-1; i++)
    {
        sub_bytes_block128(result);
        printf("sub bytes\n"); print_block128(result);

        shift_rows_block128(result);
        printf("shift rows\n"); print_block128(result);

        mix_columns_block128(result);
        printf("mix columns\n"); print_block128(result);

        xor_block128(result, &keys[i]);
        printf("add key\n"); print_block128(result);
    }

    // Last round
    sub_bytes_block128(result);
    printf("sub bytes\n"); print_block128(result);

    shift_rows_block128(result);
    printf("shift rows\n"); print_block128(result);

    xor_block128(result, &keys[AES_BLOCK128_ROUNDS]);
    printf("add key\n"); print_block128(result);
}

void aes_decrypt128(const aes_block128_t* encrypt, const aes_block128_t* key, aes_block128_t* result)
{
    // result = encrypt
    cpy_block128(result, encrypt);

    // Generate all round keys
    aes_keys_t keys;
    aes_generate_keys128(key, keys);

    // Initial round (inverse of final encryption round)
    xor_block128(result, &keys[AES_BLOCK128_ROUNDS]);
    
    inv_shift_rows_block128(result);
    
    inv_sub_bytes_block128(result);


    // Rounds AES_BLOCK128_ROUNDS - 1 down to 1
    for (int16_t i = AES_BLOCK128_ROUNDS - 1; i > 0; i--)
    {
        xor_block128(result, &keys[i]);
        
        inv_mix_columns_block128(result);
        
        inv_shift_rows_block128(result);

        inv_sub_bytes_block128(result);
    }

    // Final round (inverse of first AddRoundKey)
    xor_block128(result, &keys[0]);
}