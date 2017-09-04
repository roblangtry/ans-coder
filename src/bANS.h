#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "reader.h"
#include "writer.h"
#include "prelude_code.h"
#include "prelude.h"
#ifndef BANS_CODE
#define BANS_CODE
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
    uint32_t * freq;
    uint32_t * cumalative_freq;
    uint64_t * I_max;
    size_t m;
};


void bANS_encode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions);
void write_meta_header(FILE * input_file, struct writer * my_writer);
void process_encode_block(uint32_t * block, size_t block_size, struct writer * my_writer, struct prelude_functions * my_prelude_functions);
void process_encode(uint32_t symbol, uint64_t * state, struct block_header * header, struct output_obj * output);
void write_output(uint64_t * state, struct output_obj * output);
void write_block(uint64_t state, struct block_header * header, struct output_obj * output, struct writer * my_writer, struct prelude_functions * my_prelude_functions);
struct output_obj get_output_obj(FILE * output_file);
struct block_header calculate_block_header(uint32_t * block, size_t block_size);
void bANS_decode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions);
void process_decode_block(struct reader * my_reader, FILE * output_file, struct prelude_functions * my_prelude_functions);
void process_decode(uint64_t * state, struct block_header * header, uint32_t * output);
struct block_header read_block_header(uint64_t * state, struct reader * my_reader, struct prelude_functions * my_prelude_functions);
#endif