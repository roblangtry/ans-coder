#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "mem_manager.h"

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
    uint64_t * freq;
    uint64_t * cumalative_freq;
} file_header_t;

void preprocess_file(FILE * input_file, coding_signature_t signature, file_header_t * header);
void output_file_header(FILE * output_file, file_header_t * header, coding_signature_t signature);
void read_file_header(FILE * input_file, coding_signature_t * signature, file_header_t * header);
#endif