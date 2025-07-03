#include "dh.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ENDIAN_MPZ -1

static mpz_t p;
static mpz_t g;

// Creates a random mpz value using urandom
static void random_mpz_from_urandom(mpz_t result, uint64_t num_bytes) 
{
    FILE *f = fopen("/dev/urandom", "rb");
    
    if (!f) 
    {
        fprintf(stderr, "Failed to open /dev/urandom");
        exit(EXIT_FAILURE);
    }

    uint8_t buffer[num_bytes];
    if (fread(buffer, 1, num_bytes, f) != num_bytes) 
    {
        fprintf(stderr, "Failed to read enough random bytes");
        fclose(f);
        exit(EXIT_FAILURE);
    }
    fclose(f);

    mpz_import(result, num_bytes, ENDIAN_MPZ, 1, 0, 0, buffer);
}

static void export_mpz_fixed(uint8_t* out, const mpz_t val, uint64_t size)
{
    uint64_t count = 0;

    mpz_export(out, &count, ENDIAN_MPZ, 1, 0, 0, val);

    if (count < size)
    {
        uint64_t diff = size - count;

        // big endian
        if (ENDIAN_MPZ == 1)
        {
            memmove(out + diff, out, count);
            memset(out, 0, diff);
        }
        else if (ENDIAN_MPZ == -1)
        {
            memset(out + count, 0, diff);
        }
    }
}
// Generates a prime number with `bytes` bytes, res < (global)p
static void generate_prime_low_p(mpz_t res, int bytes)
{
    while (true) 
    {
        random_mpz_from_urandom(res, bytes);

        mpz_nextprime(res, res);

        // Found only if res < p
        if (mpz_cmp(res, p) < 0) 
        {
            break;
        }
    }
}

// Generates a prime with `bytes` bytes
static void generate_prime(mpz_t res, int bytes)
{
    random_mpz_from_urandom(res, bytes - 1);
    
    mpz_setbit(res, bytes * 8 - 1);
    mpz_setbit(res, 0);

    mpz_nextprime(res, res);
}

void init_dh()
{
    mpz_init(p);
    generate_prime(p, DH_KEY_BYTES);
    mpz_init_set_ui(g, G_GLOBAL);
}

void get_prime_dh(uint8_t res[DH_KEY_BYTES])
{
    export_mpz_fixed(res, p, DH_KEY_BYTES);
}

void get_generator_dh(uint64_t* res)
{
    *res = mpz_get_ui(g);
}


void set_prime_dh(const uint8_t value[DH_KEY_BYTES])
{
    mpz_import(p, DH_KEY_BYTES, ENDIAN_MPZ, 1, 0, 0, value);
}

void set_generator_dh(const uint64_t value)
{
    mpz_set_ui(g, value);
}


void free_dh()
{
    mpz_clear(p);
    mpz_clear(g);
}

dh_key_t create_dh_key()
{
    dh_key_t key;
    mpz_init(key.this_private);
    mpz_init(key.this_public);
    mpz_init(key.other_public);
    mpz_init(key.common_key);

    return key;
}

void gen_dh_key_private_prime(dh_key_t* key)
{
    // max 2^(DH_KEY_BYTES*8) random prime value
    generate_prime_low_p(key->this_private, DH_KEY_BYTES);

    // this_public = g^x % p
    mpz_powm(key->this_public, g, key->this_private, p);
}

void set_dh_key_private_prime(dh_key_t* key, const uint8_t val[DH_KEY_BYTES])
{
    // max 2^(DH_KEY_BYTES*8) prime value in `val`, x=`val`
    mpz_import(key->this_private, DH_KEY_BYTES, 1, 1, 1, 0, val);

    // this_public = g^x % p
    mpz_powm(key->this_public, g, key->this_private, p);
}

void get_dh_key_private_prime(const dh_key_t* key, uint8_t res[DH_KEY_BYTES])
{
    export_mpz_fixed(res, key->this_private, DH_KEY_BYTES);
}

void set_dh_key_other_public(dh_key_t* key, const uint8_t val[DH_KEY_BYTES])
{
    // Import bytes into the mpz_t
    mpz_import(key->other_public, DH_KEY_BYTES, ENDIAN_MPZ, 1, 0, 0, val);

    // common_key = (g^y (other_public)) ^ (x(private)) % p => g^xy % p
    mpz_powm(key->common_key, key->other_public, key->this_private, p);
}

void get_dh_key_other_public(const dh_key_t* key, uint8_t res[DH_KEY_BYTES])
{
    export_mpz_fixed(res, key->other_public, DH_KEY_BYTES);
}

void get_dh_common_key(const dh_key_t* key, uint8_t res[DH_KEY_BYTES])
{
    export_mpz_fixed(res, key->common_key, DH_KEY_BYTES);
}

void free_dh_key(dh_key_t* key)
{
    mpz_clear(key->this_private);
    mpz_clear(key->this_public);
    mpz_clear(key->other_public);
    mpz_clear(key->common_key);
}
