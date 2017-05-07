#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "preprocessing.h"
#include "reverse_reader.h"
#include "priority.h"
#ifndef RANS_CODE
#define RANS_CODE
#define BYTES_TO_WRITE_OUT 1
#define BUFFER_SIZE 1048576
#define OUT_BUFFER_SIZE 1048576
struct t_preamble
{
    uint64_t write_size;
    uint64_t bits_to_write;
    uint64_t I;
    uint64_t * symbol_state;
    uint64_t * cumalative_frequency;
    int64_t * aligned_frequency;
    uint64_t * I_max;
    uint64_t * I_min;
    uint64_t * state_lut;
    uint * sym_lut;
    uint64_t ** sxs;
};
struct t_buffered_writer
{
    size_t size;
    size_t max_size;
    unsigned char current_byte;
    unsigned char current_position;
    FILE * file;
    unsigned char * buffer;
};
struct t_buffered_uint_writer
{
    size_t size;
    size_t max_size;
    FILE * file;
    uint * buffer;
};
struct decode_result {
    uint64_t state;
    uint symbol;
};
void tANS_encode(FILE * input_file, FILE * output_file, struct header header);
struct t_preamble t_build_preamble(struct header header, unsigned char encode);
uint64_t t_process_symbol(uint64_t state, uint input_symbol, struct t_preamble preamble, struct header header, struct t_buffered_writer * writer);
void t_put(unsigned char byte, struct t_buffered_writer * writer);
void t_writer_flush(struct t_buffered_writer * writer);
void t_write_out(uint symbol, struct t_buffered_uint_writer * writer);
void t_write_flush(struct t_buffered_uint_writer * writer);
void tANS_decode(FILE * input_file, FILE * output_file, struct header header, unsigned char verbose_flag);
uint64_t t_calculate_state(struct header * header, struct t_preamble * preamble, uint64_t * symbol, uint64_t state);
#endif