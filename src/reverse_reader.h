#include <stdio.h>
#include <stdlib.h>
#ifndef REVERSE_CODE
#define REVERSE_CODE
#define READER_BUFFER 1048576
struct reverse_reader {
    FILE * file;
    size_t current;
    size_t size;
    size_t buffer_start;
    size_t buffer_size;
    unsigned char stop;
    uint * buffer;
};
struct decode_source {
    FILE * file;
    size_t buffer_size;
    size_t buffer_start;
    size_t content_start;
    size_t current;
    unsigned char start;
    unsigned char stop;
    unsigned char current_byte;
    unsigned char position;
    unsigned char * buffer;
};
unsigned char yield_uint(struct reverse_reader * reader, unsigned int * value);
struct reverse_reader get_reader(FILE * file);
struct decode_source get_decoder_source(FILE * file, size_t start, size_t end);
unsigned char yield_decoder_byte(struct decode_source * source);
void yield_decoder_bit(struct decode_source * source, unsigned char * value);
#endif