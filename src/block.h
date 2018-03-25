#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "mem_manager.h"
#include "file_header.h"
#include "writer.h"
#include "reader.h"
#include "prelude_code.h"
#include "msb.h"
#include "page.h"
#include "order.h"
#include "sparse_hash.h"


#ifndef BLOCK_CODE
#define BLOCK_CODE
void process_block(FILE * input_file, struct writer * my_writer, file_header_t * header, coding_signature_t signature);
void read_block(struct reader * my_reader, file_header_t * header, coding_signature_t signature, data_block_t * block);
void output_to_file(FILE * output_file, data_block_t * data);
#endif