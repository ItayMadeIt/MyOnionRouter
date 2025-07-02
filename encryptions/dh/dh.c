#include "dh.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ENDIAN_MPZ -1
#define PRIME_ROUNDS 25

static mpz_t n;
static mpz_t g;

// Creates a random mpz value using urandom
static void random_mpz_from_urandom(mpz_t result, size_t num_bytes) 
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


// Generates a prime number with `bytes` bytes, p < (global)n
static void generate_prime_low_n(mpz_t p, int bytes)
{
    while (true) 
    {
        random_mpz_from_urandom(p, bytes);

        mpz_nextprime(p, p);

        // Found only if p < n
        if (mpz_cmp(p, n) < 0) 
        {
            break;
        }
    }
}

// Generates a prime with `bytes` bytes
static void generate_prime(mpz_t p, int bytes)
{
    random_mpz_from_urandom(p, bytes - 1);
    
    mpz_setbit(p, bytes * 8 - 1);
    mpz_setbit(p, 0);

    mpz_nextprime(p, p);
}

void init_dh()
{
    mpz_init(n);
    generate_prime(n, DH_KEY_BYTES);

    mpz_init_set_ui(g, 5);
}

void init_dh_known_n(uint8_t public_n[DH_KEY_BYTES])
{
    mpz_init(n);
    mpz_import(n, DH_KEY_BYTES, ENDIAN_MPZ, 1, 0, 0, public_n);

    mpz_init_set_ui(g, G_GLOBAL);
}

void free_dh()
{
    mpz_clear(n);
    mpz_clear(g);
}

dh_key_t create_dh_key()
{
    dh_key_t key;
    mpz_init(key.this_private);
    mpz_init(key.this_public);
    mpz_init(key.other_public);
    mpz_init(key.common_key);

    // max 2^(DH_KEY_BYTES*8) random prime value
    generate_prime_low_n(key.this_private, DH_KEY_BYTES);

    // this_public = g^x % n
    mpz_powm(key.this_public, g, key.this_private, n);

    return key;
}

void export_other_public(dh_key_t* key, uint8_t other_public_data[DH_KEY_BYTES])
{
    size_t count = 0;

    mpz_export(other_public_data, &count, ENDIAN_MPZ, 1, 0, 0, key->other_public);

    for (size_t i = count; i < DH_KEY_BYTES; i++) 
    {
        other_public_data[i] = 0;
    }
}


void set_dh_key_other_public(dh_key_t* key, uint8_t other_public_data[DH_KEY_BYTES])
{
    // Import bytes into the mpz_t
    mpz_import(key->other_public, DH_KEY_BYTES, ENDIAN_MPZ, 1, 0, 0, other_public_data);

    // common_key = (g^y (other_public)) ^ (x(private)) % n => g^xy % n
    mpz_powm(key->common_key, key->other_public, key->this_private, n);
}

void free_dh_key(dh_key_t* key)
{
    mpz_clear(key->this_private);
    mpz_clear(key->this_public);
    mpz_clear(key->other_public);
    mpz_clear(key->common_key);
}
