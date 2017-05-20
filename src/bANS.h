#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef RANS_CODE
#define RANS_CODE
#define BLOCK_LEN 131072
#define SYMBOL_RANGE 524288
#define B 256
#define Bbits 8
struct output_obj
{
    unsigned char * output;
    size_t head;
    FILE * file;
};
struct block_header
{
    size_t * index;
    size_t no_symbols;
    size_t content_length;
    size_t block_len;
    size_t  * symbol_state;
    uint32_t * symbol;
    uint64_t * freq;
    uint64_t * cumalative_freq;
    uint64_t * I_max;
    size_t m;
};


void bANS_encode(FILE * input_file, FILE * output_file);
void write_meta_header(FILE * input_file, FILE * output_file);
void process_encode_block(uint32_t * block, size_t block_size, FILE * output_file);
void process_encode(uint32_t symbol, uint64_t * state, struct block_header * header, struct output_obj * output);
void write_output(uint64_t * state, struct output_obj * output);
void write_block(uint64_t state, struct block_header * header, struct output_obj * output);
struct output_obj get_output_obj(FILE * output_file);
struct block_header calculate_block_header(uint32_t * block, size_t block_size);
void bANS_decode(FILE * input_file, FILE * output_file);
void process_decode_block(FILE * input_file, FILE * output_file);
void process_decode(uint64_t * state, struct block_header * header, uint32_t * output);
struct block_header read_block_header(FILE * input_file);
#endif