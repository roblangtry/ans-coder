#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mem_manager.h"
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
    unsigned int buffer_size;
    unsigned int buffer_start;
    unsigned int content_start;
    unsigned int current;
    unsigned char start;
    unsigned char stop;
    unsigned int current_byte;
    unsigned int position;
    unsigned char * buffer;
};
unsigned char yield_uint(struct reverse_reader * reader, unsigned int * value);
struct reverse_reader get_reader(FILE * file);
struct decode_source get_decoder_source(FILE * file, size_t start, size_t end);
unsigned char yield_decoder_byte(struct decode_source * source);
void yield_decoder_bit(struct decode_source * source, unsigned int * value);
void get_bit(unsigned int * cur, unsigned int * pos, struct decode_source * source, unsigned int * value);
void get_byte(unsigned int * cur, unsigned int * pos, unsigned int * ind, unsigned int * size, unsigned char * buffer, struct decode_source * source);
void reverse_read_bytes(unsigned int * ind, unsigned int * start, unsigned int * content, unsigned int * size, unsigned char * buffer, FILE * file);
#endif