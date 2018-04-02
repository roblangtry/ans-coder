#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "mem_manager.h"
#include "writer.h"
#include "reader.h"
#include "prelude_code.h"
#include "msb.h"
#include "sparse_hash.h"

#ifndef FILE_HEADER_CODE
#define FILE_HEADER_CODE
typedef struct
{
    uint32_t * data;
    size_t size;
} data_block_t;

typedef struct
{
    uint32_t no_blocks;
    uint64_t symbols;
    uint64_t unique_symbols;
    uint32_t max;
    uint32_t global_max;
    uint32_t translation_mechanism;
    uint32_t translate_k;
    sparse_hash_t * freq_hash;
    uint32_t * freq;
    uint32_t * cumalative_freq;
    uint32_t * symbol;
    uint32_t * data;
    uint32_t * symbol_state;
    uint32_t * translation;
} file_header_t;

void preprocess_file(FILE * input_file, coding_signature_t signature, file_header_t * header);
void output_file_header(struct writer * my_writer, file_header_t * header, coding_signature_t signature);
void read_signature(struct reader * my_reader, coding_signature_t * signature, struct prelude_code_data * metadata);
void read_file_header(struct reader * my_reader, coding_signature_t signature, file_header_t * header, struct prelude_code_data * metadata);
#endif