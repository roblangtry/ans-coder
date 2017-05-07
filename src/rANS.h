#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "preprocessing.h"
#include "reverse_reader.h"
#ifndef RANS_CODE
#define RANS_CODE
#define BYTES_TO_WRITE_OUT 1
#define BITS_TO_WRITE_OUT 1
#define BUFFER_SIZE 1048576
#define OUT_BUFFER_SIZE 1048576
struct preamble
{
    uint64_t write_size;
    uint64_t bits_to_write;
    uint64_t I;
    uint64_t * symbol_state;
    uint64_t * cumalative_frequency;
    uint64_t * I_max;
    uint64_t * I_min;
    uint64_t ** ls_lut;
};
struct buffered_writer
{
    size_t size;
    size_t max_size;
    FILE * file;
    unsigned char byte;
    unsigned char len;
    unsigned char * buffer;
};
struct buffered_uint_writer
{
    size_t size;
    size_t max_size;
    FILE * file;
    uint * buffer;
};
void rANS_encode(FILE * input_file, FILE * output_file, struct header header);
struct preamble build_preamble(struct header header);
uint64_t process_symbol(uint64_t state, uint input_symbol, struct preamble preamble, struct header header, struct buffered_writer * writer);
uint64_t pprocess_symbol(uint64_t state, uint input_symbol, struct preamble preamble, struct header header, struct buffered_writer * writer);
void put(unsigned char byte, struct buffered_writer * writer);
void writer_flush(struct buffered_writer * writer);
void write_out(uint symbol, struct buffered_uint_writer * writer);
void write_flush(struct buffered_uint_writer * writer);
void rANS_decode(FILE * input_file, FILE * output_file, struct header header, unsigned char verbose_flag);
inline void calculate_state(struct header * header, struct preamble * preamble, uint64_t * symbol, uint64_t * state);
#endif