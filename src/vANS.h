#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "reader.h"
#include "writer.h"
#include "prelude_code.h"
#include "prelude.h"
#include "bANS.h"
#include "constants.h"
#ifndef VANS_CODE
#define VANS_CODE
typedef struct {
    uint32_t * data;
    size_t length;
} vector_t;
void write_vector(struct prelude_code_data * metadata, vector_t * vector);
vector_t * read_vector(struct prelude_code_data * metadata);
void add_to_vector(vector_t * vector, uint32_t number);
vector_t compare_vectors(vector_t vector_a, vector_t vector_b);
vector_t * get_blank_vector();
unsigned char check_vector(vector_t * vector, uint32_t number);
uint32_t iterate_vector(uint32_t hot_start, vector_t * vector);
void vANS_encode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions);
void write_vector_meta_header(FILE * input_file, struct writer * my_writer);
vector_t * prepare_common_vector(FILE * input_file, struct writer * my_writer);
void vANS_decode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions);
void process_vector_decode_block(struct reader * my_reader, FILE * output_file, struct prelude_functions * my_prelude_functions);
struct block_header read_vector_block_header(uint64_t * state, struct reader * my_reader, struct prelude_functions * my_prelude_functions);
void process_encode_vector_block(uint32_t * block, size_t block_size, struct writer * my_writer, struct prelude_functions * my_prelude_functions);
void write_vector_block(uint64_t state, struct block_header * header, struct output_obj * output, struct writer * my_writer, struct prelude_functions * my_prelude_functions);

void write_vector_symbol_prelude(uint32_t * symbols, uint32_t * symbol_frequencies, size_t * no_symbols, uint64_t * state, size_t * content_length, struct writer * my_writer, struct prelude_functions * my_prelude_functions);
void read_vector_symbol_prelude(size_t * no_symbols, uint32_t ** symbols, uint32_t ** symbol_frequencies, uint64_t * state, size_t * content_length, struct reader * my_reader, struct prelude_functions * my_prelude_functions);

#endif