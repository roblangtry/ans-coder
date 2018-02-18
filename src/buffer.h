#include <stdint.h>
#include <stdio.h>
#include "constants.h"
#include "mem_manager.h"
#ifndef BUFFER_CODE
#define BUFFER_CODE
typedef struct {
    uint64_t current;
    uint32_t read;
    uint32_t clen;
    uint32_t blen;
    uint32_t r1;
    uint32_t r2;
    uint32_t buffer[BUFFER_SZ];
} bit_buffer;

void start_buffer(bit_buffer * buffer);
void buffer_bit(int bit, bit_buffer * buffer);
void buffer_bits(int bits, int len, bit_buffer * buffer);
void set_buffer(uint32_t value, uint32_t len, bit_buffer * buffer);
void buffer_int(uint32_t value, bit_buffer * buffer);
uint32_t get_buffered_bit(bit_buffer * buffer);
#endif