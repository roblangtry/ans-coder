#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "preprocessing.h"
#ifndef RANS_CODE
#define RANS_CODE
#define BYTES_TO_WRITE_OUT 1
#define BUFFER_SIZE 4096
#define OUT_BUFFER_SIZE 1048576
struct preamble
{
    uint64_t write_size;
    uint64_t * symbol_state;
    uint64_t * cumalative_frequency;
    uint64_t * I_max;
};
struct buffered_writer
{
    size_t size;
    size_t max_size;
    FILE * file;
    unsigned char * buffer;
};
rANS_encode(FILE * input_file, FILE * output_file, struct header header);
struct preamble build_preamble(struct header header);
uint64_t process_symbol(uint64_t state, uint input_symbol, struct preamble preamble, struct header header, struct buffered_writer * writer);
void put(unsigned char byte, struct buffered_writer * writer);
void writer_flush(struct buffered_writer * writer);
#endif