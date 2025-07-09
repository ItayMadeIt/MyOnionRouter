#ifndef __ENCRYPTIONS_H__
#define __ENCRYPTIONS_H__

#include <aes/aes.h>
#include <dh/dh.h>
#include <sha/sha256.h>

#define ASYMMETRIC_KEY_BYTES 256 /*Same as diffie hellman 2^256*8 = 2^2048, size of key*/
#define SHA_PADDING 64 
#define SHA_BUF_SIZE 32 
#define AES_BYTES 16 

typedef struct key_data
{
    dh_key_t asymmetric_key;
    aes_block128_t symmetric_key; 
} key_data_t;

void init_encryption();
void free_encryption();

// If none globals are specified using set_globals, valid ones will be generated
void get_globals(uint64_t* g/*less than 256*/, uint8_t p[ASYMMETRIC_KEY_BYTES]);

// Load globals from raw ones using bytes
void set_globals(const uint64_t g/*less than 256*/, const uint8_t p[ASYMMETRIC_KEY_BYTES]);

// Allocate space for key
key_data_t init_key();

// Frees space for key
void free_key(key_data_t* key);

// Generates private assymetric keys
void gen_asymmetric_private_key(key_data_t* key);
// Sets the private key x, and calculates g^x
void set_asymmetric_private_key(key_data_t* key, const uint8_t private_key[ASYMMETRIC_KEY_BYTES]);

// Sets g^y and g^xy, sets symmetric key for aes using sha256(g^xy)
void derive_symmetric_key_from_public(key_data_t* key, uint8_t data[ASYMMETRIC_KEY_BYTES]);

// Uses symmetric key to encrypt data, result is in data as well...
void symmetric_encrypt(key_data_t* key, uint8_t* data, uint64_t length);


#endif // __ENCRYPTIONS_H__