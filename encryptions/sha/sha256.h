#include <stdint.h>
#include <stdbool.h>

#define BYTE_TO_BIT 8
#define BLOCK_SIZE_BITS 512
#define BLOCK_SIZE_BYTES (BLOCK_SIZE_BITS / BYTE_TO_BIT)

uint64_t hash_prepare_data_sha256(uint8_t* data, uint64_t length, uint64_t max_len);

// The input 'data' MUST be pre-padded to a multiple of 64 bytes (512 bits).
// This function will not perform padding or length encoding.
void hash_sha256(uint8_t* data , uint64_t length, uint8_t* out_hash /*32 bytes*/);
