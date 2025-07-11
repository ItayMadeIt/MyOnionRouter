#include "encryptions.h"
#include "dh/dh.h"
#include <sha/sha256.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_encryption()
{
    init_dh();
}

void free_encryption()
{
    free_dh();
}

// If none globals are specified using load_globals, random ones will be generated
void get_globals(uint64_t* g/*less than 256*/, uint8_t p[ASYMMETRIC_KEY_BYTES])
{
    get_prime_dh(p);
    get_generator_dh(g);
}

// Set globals from raw ones using bytes
void set_globals(const uint64_t g/*less than 256*/, const uint8_t p[ASYMMETRIC_KEY_BYTES])
{
    set_prime_dh(p);
    set_generator_dh(g);
}

// Allocates identity key
void init_id_key(identity_key_t* id_key)
{
    create_dh_identity_key(id_key);
    gen_dh_prime_identity_key(id_key);
}

void free_id_key(identity_key_t* id_key)
{
    free_dh_identity_key(id_key);
}

// Allocate space for key
void init_key(key_data_t* key, const identity_key_t* identity)
{
    key->identity = identity;

    create_dh_session_key(&key->session, identity);

    memset(&key->symmetric_key, 0, sizeof(key->symmetric_key));
}

// Frees space for key
void free_key(key_data_t* key)
{
    free_dh_session_key(&key->session);
    memset(&key->symmetric_key, 0, sizeof(key->symmetric_key));
}

// Sets g^y and g^xy, sets symmetric key for aes using sha256(g^xy)
void derive_symmetric_key_from_public(key_data_t* key, uint8_t data[ASYMMETRIC_KEY_BYTES])
{
    // Set g^y and compute g^xy in session
    set_dh_key_other_public(&key->session, data);

    uint8_t common_key[ASYMMETRIC_KEY_BYTES];

    get_dh_common_key(&key->session, common_key);

    uint8_t common_key_sha_buffer[ASYMMETRIC_KEY_BYTES + SHA_PADDING];
    
    uint64_t common_key_sha_buffer_len = 
            hash_prepare_data_sha256(common_key_sha_buffer, ASYMMETRIC_KEY_BYTES, sizeof(common_key_sha_buffer));

    if (common_key_sha_buffer_len == 0 || common_key_sha_buffer_len > sizeof(common_key_sha_buffer))
    {
        fprintf(stderr, "Common key couldn't be hashed in `derive_symmetric_key_from_public`: %lu\n", common_key_sha_buffer_len);
        abort();
    }

    uint8_t sha_buffer[SHA_BUF_SIZE];

    hash_sha256(common_key_sha_buffer, common_key_sha_buffer_len, sha_buffer);

    memcpy(&key->symmetric_key, sha_buffer, AES_BYTES);
}

// Uses symmetric key to encrypt data, result is in data as well...
void symmetric_encrypt(key_data_t* key, uint8_t* data, uint64_t length)
{
    uint64_t index = 0;

    while (index + AES_BYTES < length)
    {
        aes_block128_t aes_data;
        aes_block128_t aes_encrypt;
        
        memcpy(&aes_data, data + index, AES_BYTES);

        aes_encrypt128(&aes_data, &key->symmetric_key, &aes_encrypt);

        memcpy(data + index, &aes_encrypt, AES_BYTES);

        index += AES_BYTES;
    }

    uint16_t remainder = length - index;
    if (remainder == 0)
    {
        return;
    }

    aes_block128_t aes_data;
    memset(&aes_data, 0, sizeof(aes_data));
    
    aes_block128_t aes_encrypt;
    memset(&aes_encrypt, 0, sizeof(aes_encrypt));
    memcpy(&aes_data, data + index, remainder);

    aes_encrypt128(&aes_data, &key->symmetric_key, &aes_encrypt);

    memcpy(data + index, &aes_encrypt, remainder);
}

// Uses symmetric key to decrypt data, result is in data
void symmetric_decrypt(key_data_t* key, uint8_t* data, uint64_t length)
{
    uint64_t index = 0;

    while (index + AES_BYTES < length)
    {
        aes_block128_t aes_data;
        aes_block128_t aes_decrypt;

        memcpy(&aes_data, data + index, AES_BYTES);

        aes_decrypt128(&aes_data, &key->symmetric_key, &aes_decrypt);

        memcpy(data + index, &aes_decrypt, AES_BYTES);

        index += AES_BYTES;
    }

    uint16_t remainder = length - index;
    if (remainder == 0)
    {
        return;
    }

    aes_block128_t aes_data;
    memset(&aes_data, 0, sizeof(aes_data));

    aes_block128_t aes_decrypt;
    memset(&aes_decrypt, 0, sizeof(aes_decrypt));

    memcpy(&aes_data, data + index, remainder);

    aes_decrypt128(&aes_data, &key->symmetric_key, &aes_decrypt);

    memcpy(data + index, &aes_decrypt, remainder);
}


