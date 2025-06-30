#include <aes.h>

static uint32_t round_constants[AES_BLOCK128_ROUNDS] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000, 
    0x10000000, 0x20000000, 0x40000000, 0x80000000, 
    0x1B000000, 0x36000000
};

static inline uint32_t sub_word(uint32_t word) 
{
    return (sbox[word >> 24] << 24) |
           (sbox[(word >> 16) & 0xFF] << 16) |
           (sbox[(word >> 8)  & 0xFF] << 8) |
           (sbox[word & 0xFF]);
}

static inline uint32_t rot_word(uint32_t word)
{
    return (word << 8) | (word >> 24);
}

void aes_generate_keys128(const aes_block128_t* base_key, aes_block128_t* keys)
{
    // keys[0] = base_key
    cpy_block128(keys, base_key);

    // Start with the second aes_block_t*
    uint32_t* words = (uint32_t*)keys;

    const uint8_t WORDS_IN_BLOCK = sizeof(uint128_t) / sizeof(uint32_t);

    for (uint32_t i = WORDS_IN_BLOCK; i < AES_BLOCK128_SIZE * WORDS_IN_BLOCK; i++)
    {
        // Divides by 4
        if (i % 4 == 0)
        {
            words[i] = words[i-4] ^ sub_word(rot_word(words[i-1])) ^ round_constants[i>>2];
        }
        else
        {
            words[i] = words[i-4] ^ words[i-1];
        }
    }
}