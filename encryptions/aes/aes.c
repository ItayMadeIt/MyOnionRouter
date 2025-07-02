#include <aes/aes.h>

void aes_encrypt128(const aes_block128_t* data   , const aes_block128_t* key, aes_block128_t* result)
{
    // result = data 
    cpy_block128(result, data);

    // Generate keys
    aes_keys_t keys;
    aes_generate_keys128(key, keys);

    // Apply first round key
    xor_block128(result, &keys[0]);

    // Rounds 1 to AES_BLOCK128_ROUNDS-1
    for (uint16_t i = 1; i <= AES_BLOCK128_ROUNDS-1; i++)
    {
        sub_bytes_block128(result);

        shift_rows_block128(result);

        mix_columns_block128(result);

        xor_block128(result, &keys[i]);
    }

    // Last round
    sub_bytes_block128(result);

    shift_rows_block128(result);

    xor_block128(result, &keys[AES_BLOCK128_ROUNDS]);
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