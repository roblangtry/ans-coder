#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#ifndef READER_CODE
#define READER_CODE
#define READ_BUFFER 1048576
struct reader
{
	unsigned char buffer[READ_BUFFER];
    size_t index;
    size_t size;
    FILE * input_file;
};
struct bit_reader
{
    struct reader * my_reader;
    uint64_t buffer;
    unsigned char length;
};
//reader functions
struct reader * initialise_reader(FILE * input_file);
size_t read_byte(unsigned char * target, struct reader * my_reader);
size_t read_bytes(unsigned char * target, size_t no_bytes, struct reader * my_reader);
//bit reader functions
struct bit_reader * initialise_bit_reader(struct reader * my_reader);
uint64_t read_bits(uint64_t length, struct bit_reader * my_bit_reader);
#endif
