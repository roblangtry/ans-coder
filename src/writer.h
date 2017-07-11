#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#ifndef WRITER_CODE
#define WRITER_CODE
#define WRITE_BUFFER 1048576
struct writer
{
    unsigned char buffer[WRITE_BUFFER];
    size_t index;
    FILE * output_file;
};
struct bit_writer
{
    struct writer * my_writer;
    uint64_t buffer;
    unsigned char length;
};
//writer functions
struct writer * initialise_writer(FILE * output_file);
uint64_t write_byte(unsigned char byte, struct writer * my_writer);
uint64_t write_bytes(unsigned char * byte_array, size_t no_bytes, struct writer * my_writer);
uint64_t flush_writer(struct writer * my_writer);
//bit writer functions
struct bit_writer * initialise_bit_writer(struct writer * my_writer);
uint64_t write_bits(uint64_t value, uint64_t length, struct bit_writer * my_bit_writer);
uint64_t flush_bit_writer(struct bit_writer * my_bit_writer);
#endif