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


void init_dh();
void init_dh_known_n(uint8_t public_n[DH_KEY_BYTES]);
void free_dh();

dh_key_t create_dh_key();

void set_dh_key_other_public(dh_key_t* key, uint8_t other_public_data[DH_KEY_BYTES]);

void free_dh_key(dh_key_t* key);
