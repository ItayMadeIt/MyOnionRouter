#ifndef __ENCRYPTIONS_H__
#define __ENCRYPTIONS_H__

#include "aes.h"
#include "dh.h"

#define ASYMMETRIC_KEY_BYTES 256 /*Same as diffie hellman 2^256*8 = 2^2048, size of key*/
#define SHA_PADDING 64 
#define SHA_BUF_SIZE 32 
#define SYMMETRIC_KEY_BYTES 16 

typedef struct global_data
{
    uint8_t p[ASYMMETRIC_KEY_BYTES];
    uint64_t g;
} global_data_t;

typedef dh_identity_key_t identity_key_t; 

typedef struct key_data
{
    const identity_key_t* identity;  // Shared, external identity key
    dh_session_key_t session;           // Unique per connection
    aes_block128_t symmetric_key;
} key_data_t;

// temp
void print_block(const aes_block128_t* block);
void print_asymmertic(const uint8_t data[ASYMMETRIC_KEY_BYTES]);

void init_encryption();
void free_encryption();

// If none globals are specified using set_globals, valid ones will be generated
void get_globals(uint64_t* g/*less than 256*/, uint8_t p[ASYMMETRIC_KEY_BYTES]);

// Load globals from raw ones using bytes
void set_globals(const uint64_t g/*less than 256*/, const uint8_t p[ASYMMETRIC_KEY_BYTES]);

// Initializes identity key and generates x, g^x
void init_id_key(identity_key_t* id_key);

// Frees identity key
void free_id_key(identity_key_t* id_key);

// Allocate space for key, with ref to x,g^x
void init_key(key_data_t* key, const identity_key_t* identity);

// Frees space for key
void free_key(key_data_t* key);

// Get g^x from identity key
void get_public_identity_key(const identity_key_t *key, uint8_t res[DH_KEY_BYTES]);

// Get g^y from session key (other public key)
void get_other_public_key(const key_data_t* key, uint8_t res[DH_KEY_BYTES]);

// Sets g^y(param) and g^xy, sets symmetric key for aes using sha256(g^xy)
void derive_symmetric_key_from_public(key_data_t* key, const uint8_t data[ASYMMETRIC_KEY_BYTES]);

// Uses symmetric key to encrypt data, result is in data as well...
void symmetric_encrypt(key_data_t* key, uint8_t* data, uint64_t length);

// Uses symmetric key to decrypt data, result is in data as well...
void symmetric_decrypt(key_data_t* key, uint8_t* data, uint64_t length);


#endif // __ENCRYPTIONS_H__