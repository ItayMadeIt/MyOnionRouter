#include <sha/sha256.h>
#include <assert.h>

uint32_t round_constants[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};


bool hash_prepare_data_sha256(uint8_t* data, uint64_t length, uint64_t max_len)
{
    // Calculate total padded length
    uint64_t total_len = length + 1 + sizeof(uint64_t);
    uint64_t pad_len = (BLOCK_SIZE_BYTES - (total_len % BLOCK_SIZE_BYTES)) % BLOCK_SIZE_BYTES;
    total_len += pad_len;
    
    if (total_len > max_len)
    {
        return false;
    }

    // Append 0x80 byte
    data[length] = 0x80;

    // Zero fill until last 8 bytes
    memset(data + length + 1, 0, pad_len);

    // Append length in bits (big-endian) at the end
    uint64_t bitlen = length * BYTE_TO_BIT;
    for(int i = 0; i < sizeof(uint64_t); ++i) 
    {
        data[total_len - sizeof(uint64_t) + i] = (bitlen >> (8 * (7 - i))) & 0xFF;
    }

    return total_len <= max_len;
}

static inline uint32_t rotr(uint32_t x, uint32_t n) 
{
    return (x >> n) | (x << (32 - n));
}

static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) 
{
    return (x & y) ^ (~x & z);
}

static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) 
{
    return (x & y) ^ (x & z) ^ (y & z);
}

static inline uint32_t bsig0(uint32_t x) 
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

static inline uint32_t bsig1(uint32_t x) 
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

static inline uint32_t ssig0(uint32_t x) 
{
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

static inline uint32_t ssig1(uint32_t x) 
{
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

void hash_sha256(uint8_t* data , uint64_t length, uint8_t* out_hash /*32 bytes*/)
{
    /*Must be a multiple of BLOCK_SIZE*/
    assert(length % BLOCK_SIZE_BYTES == 0);

    uint32_t h0 = 0x6a09e667;
    uint32_t h1 = 0xbb67ae85;
    uint32_t h2 = 0x3c6ef372;
    uint32_t h3 = 0xa54ff53a;
    uint32_t h4 = 0x510e527f;
    uint32_t h5 = 0x9b05688c;
    uint32_t h6 = 0x1f83d9ab;
    uint32_t h7 = 0x5be0cd19;

    uint64_t num_blocks = length / BLOCK_SIZE_BYTES;

    for (uint64_t block_index = 0; block_index < num_blocks; ++block_index) 
    {
        const uint8_t* block = data + block_index*BLOCK_SIZE_BYTES;
        uint32_t w[64];

        // Prepare message schedule
        for (int i = 0; i < 16; ++i) 
        {
            w[i] = (block[i*4+0] << 24) | (block[i*4+1] << 16) |
                   (block[i*4+2] << 8)  | (block[i*4+3]);
        }

        for (int i = 16; i < 64; ++i) 
        {
            w[i] = ssig1(w[i-2]) + w[i-7] + ssig0(w[i-15]) + w[i-16];
        }

        // iteration vars
        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6, h = h7;

        // Compression function main loop
        for (int i = 0; i < 64; ++i) 
        {
            uint32_t T1 = h + bsig1(e) + ch(e, f, g) + round_constants[i] + w[i];
            uint32_t T2 = bsig0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        // Add the compressed chunk to the current hash value
        h0 += a; h1 += b; h2 += c; h3 += d;
        h4 += e; h5 += f; h6 += g; h7 += h;
    }

    uint32_t h_values[8] = { h0, h1, h2, h3, h4, h5, h6, h7 };
    for (uint32_t i = 0; i < 8; ++i) 
    {
        out_hash[i*4+0] = (h_values[i] >> 24) & 0xff;
        out_hash[i*4+1] = (h_values[i] >> 16) & 0xff;
        out_hash[i*4+2] = (h_values[i] >> 8 ) & 0xff;
        out_hash[i*4+3] = (h_values[i]      ) & 0xff;
    }
}