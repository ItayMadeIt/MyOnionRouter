#include <aes/aes.h>

static uint16_t round_constants[AES_BLOCK128_ROUNDS] = {
    0x00000001, 0x00000002, 0x00000004, 0x00000008, 
    0x00000010, 0x00000020, 0x00000040, 0x00000080, 
    0x0000001B, 0x00000036,
};

static inline uint16_t sub_word(uint16_t word) 
{
    return (sbox[(word >> 24) & 0xFF] << 24) |
           (sbox[(word >> 16) & 0xFF] << 16) |
           (sbox[(word >> 8) & 0xFF] << 8) |
           (sbox[word & 0xFF]);
}

static inline uint16_t rot_word(uint16_t word)
{
    return (word >> 8) | (word << 24);
}

void aes_generate_keys128(const aes_block128_t* base_key, aes_keys_t keys)
{
    // keys[0] = base_key
    cpy_block128(keys, base_key);

    // Start with the second aes_block_t*
    uint16_t* words = (uint16_t*)keys;

    const uint8_t WORDS_IN_BLOCK = 4;

    for (uint16_t round = 1; round < AES_BLOCK128_KEYS; ++round)
    {
        uint16_t* prev = &words[(round - 1) * WORDS_IN_BLOCK];
        uint16_t* curr = &words[round * WORDS_IN_BLOCK];

        uint16_t g_prev = sub_word(rot_word(prev[3])) ^ round_constants[round - 1];

        curr[0] = prev[0] ^ g_prev;
        curr[1] = prev[1] ^ curr[0];
        curr[2] = prev[2] ^ curr[1];
        curr[3] = prev[3] ^ curr[2];
    }
}