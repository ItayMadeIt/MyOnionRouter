#ifndef __DH_H__
#define __DH_H__

#include <gmp.h>
#include <stdint.h>

#define DH_KEY_BYTES 256
#define G_GLOBAL 5

typedef struct dh_key
{
    mpz_t this_private; // x
    mpz_t this_public ; // g^x

    mpz_t other_public; // g^y
    mpz_t common_key  ; // g^xy

} dh_key_t;

void init_dh(void);
void free_dh(void);

void get_prime_dh(uint8_t res[DH_KEY_BYTES]);
void set_prime_dh(const uint8_t value[DH_KEY_BYTES]);
void get_generator_dh(uint64_t* res);
void set_generator_dh(uint64_t value);

dh_key_t create_dh_key(void);
void free_dh_key(dh_key_t* key);

void gen_dh_key_private_prime(dh_key_t* key);
void set_dh_key_private_prime(dh_key_t* key, const uint8_t val[DH_KEY_BYTES]);
void get_dh_key_private_prime(const dh_key_t* key, uint8_t res[DH_KEY_BYTES]);

void set_dh_key_other_public(dh_key_t* key, const uint8_t val[DH_KEY_BYTES]);
void get_dh_key_other_public(const dh_key_t* key, uint8_t res[DH_KEY_BYTES]);
void get_dh_common_key(const dh_key_t* key, uint8_t res[DH_KEY_BYTES]);

#endif // __DH_H__