#ifndef __DH_H__
#define __DH_H__

#include <gmp.h>
#include <stdint.h>

#define DH_KEY_BYTES 256
#define G_GLOBAL 5

typedef struct dh_identity_key {

    mpz_t this_private;
    mpz_t this_public;

} dh_identity_key_t;

typedef struct dh_session_key {

    const dh_identity_key_t* identity;
    mpz_t other_public;
    mpz_t common_key;

} dh_session_key_t;


void init_dh(void);
void free_dh(void);

void get_prime_dh(uint8_t res[DH_KEY_BYTES]);
void set_prime_dh(const uint8_t value[DH_KEY_BYTES]);
void get_generator_dh(uint64_t* res);
void set_generator_dh(uint64_t value);

void create_dh_identity_key(dh_identity_key_t* id_key);
void create_dh_session_key(dh_session_key_t* session_key, const dh_identity_key_t* identity);
void free_dh_identity_key(dh_identity_key_t* key);
void free_dh_session_key(dh_session_key_t* key);

void gen_dh_prime_identity_key(dh_identity_key_t* key);
void set_dh_prime_identity_key(dh_identity_key_t* key, const uint8_t val[DH_KEY_BYTES]);
void get_dh_public_identity_key(const dh_identity_key_t* key, uint8_t res[DH_KEY_BYTES]);

void set_dh_key_other_public(dh_session_key_t* key, const uint8_t val[DH_KEY_BYTES]);
void get_dh_common_key(const dh_session_key_t* key, uint8_t res[DH_KEY_BYTES]);

#endif // __DH_H__