#ifndef __AES_H__
#define __AES_H__

#include <stdint.h>
#include <stdbool.h>

#define BYTE_TO_BIT 8

#define AES_BLOCK128_SIZE   16
#define AES_BLOCK128_SIDE   4
#define AES_BLOCK128_ROUNDS 10

typedef struct __attribute__((aligned(16))) {
    
    union
    {
        uint8_t bytes[AES_BLOCK128_SIDE][AES_BLOCK128_SIDE];
        uint32_t rows[4];      // reinterpret each row
    }

} aes_block128_t;

typedef aes_block128_t aes_key[AES_BLOCK128_ROUNDS];

// utils
extern uint8_t sbox[BYTE_VALUES];
extern uint8_t inv_sbox[BYTE_VALUES];

void cpy_block128(restrict aes_block128_t* value, const restrict aes_block128_t* operand);
void xor_block128(restrict aes_block128_t* value, const restrict aes_block128_t* operand);

void sub_bytes_block128    (restrict aes_block128_t* value);
void inv_sub_bytes_block128(restrict aes_block128_t* value);

// generate keys
void aes_generate_keys128(const aes_block128_t* base_key, aes_keys128* keys);

// encrypt/decrypt
void aes_encrypt128(const aes_block128_t* data   , const aes_block128_t* key, aes_block128_t* result);
void aes_decrypt128(const aes_block128_t* encrypt, const aes_block128_t* key, aes_block128_t* result); 

#endif // __AES_H__