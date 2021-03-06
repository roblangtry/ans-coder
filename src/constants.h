#include <stdint.h>

#ifndef CONSTANTS_CODE
#define CONSTANTS_CODE
#define SYMBOL_MAP_SIZE 1524288
#define SPLIT_LENGTH 20
#define BLOCK_SIZE 131072
#define INITIAL_PAGES 1024
#define PAGE_SIZE 1024
#define BUFFER_SZ 1048576
#define PRE_SIZE 131072
#define CONTENT_SIZE 131072
#define POST_SIZE 131072
#define WRITE_BUFFER 131072
#define READ_BUFFER 131072
#define DONE 123456
#define MAGIC 19931207
#define MSB_BITS 8
#define MAGIC_LENGTH 32
#define RANGE_METHOD 0
#define BLOCK_METHOD 2
#define PARRALEL_METHOD 1
#define TABLE_METHOD 3
#define VECTOR_METHOD 4
#define ESCAPE_METHOD 5
#define SPLIT_METHOD 6
#define SINGLE_HEADER_METHOD 7
#define MSB_METHOD 8
#define DEBUG_FLAG 1
#define HASHING_FACTOR 10

//Methods for encoding/decoding
//Symbol
#define SYMBOL_DIRECT 0
#define SYMBOL_SPLIT 1
#define SYMBOL_MSB 2
#define SYMBOL_MSB_2 3
//Header
#define HEADER_SINGLE 0
#define HEADER_BLOCK 1
//encoding
#define ANS_RANGE 0
// bit factor
#define DEFAULT_BIT_FACTOR 13
#define DEFAULT_MSB_BIT_FACTOR 8
// translation
#define TRANSLATE_PARTIAL 3
#define TRANSLATE_TRUE 2
#define TRANSLATE_FALSE 1
#define TRANSLATE_PERMUTATION_TRUE 4
#define TRANSLATE_PERMUTATION_PARTIAL 5
// hashing
#define HASHING_STANDARD 1
#define HASHING_SPARSE 2

typedef struct
{
    uint32_t symbol;
    uint32_t header;
    uint32_t ans;
    uint32_t bit_factor;
    uint32_t msb_bit_factor;
    uint32_t translation;
    uint32_t translate_k;
    uint32_t hashing;
} coding_signature_t;

coding_signature_t get_signature();
#endif