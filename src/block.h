#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "mem_manager.h"
#include "file_header.h"

#ifndef BLOCK_CODE
#define BLOCK_CODE
typedef struct
{
    uint32_t * pre;
    uint32_t pre_size;
    uint32_t pre_max_size;
    uint32_t * content;
    uint32_t content_size;
    uint32_t content_max_size;
    uint32_t * post;
    uint32_t post_size;
    uint32_t post_max_size;
} output_block_t;

void process_block(FILE * input_file, file_header_t * header, coding_signature_t signature, output_block_t * out_block);
void output_block(FILE * output_file, output_block_t * block);
void read_block(FILE * input_file, file_header_t * header, coding_signature_t signature, data_block_t * block);
void output_to_file(FILE * output_file, data_block_t data);
#endif