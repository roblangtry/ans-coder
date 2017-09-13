#include <stdint.h>
#include <stdio.h>
#include "constants.h"
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
static inline void
start_buffer(bit_buffer * buffer)
{
    buffer->current = 0;
    buffer->clen = 0;
    buffer->blen = 0;
}
static inline void
buffer_bit(int bit, bit_buffer * buffer)
{

    buffer->current = (buffer->current << 1) + bit;
    buffer->clen++;
    if(buffer->clen == 32)
    {
        buffer->buffer[buffer->blen++] = buffer->current;
        buffer->current = 0;
        buffer->clen = 0;
    }
}
static inline void
buffer_bits(int bits, int len, bit_buffer * buffer)
{
    buffer->current = (buffer->current << len) + bits;
    buffer->clen += len;
    while(buffer->clen >= 32)
    {
        buffer->clen -= 32;
        buffer->buffer[buffer->blen] = buffer->current >> buffer->clen;
        buffer->current -= buffer->buffer[buffer->blen++] << buffer->clen;
    }

}
static inline void
set_buffer(uint32_t value, uint32_t len, bit_buffer * buffer)
{
    buffer->clen = len;
    buffer->current = value;
}
static inline void
buffer_int(uint32_t value, bit_buffer * buffer)
{
    buffer->buffer[buffer->blen++] = value;
}

static inline uint32_t get_buffered_bit(bit_buffer * buffer)
{
    uint32_t value = buffer->current & 1;
    if(!buffer->clen){
        buffer->blen--;
        buffer->current = buffer->buffer[buffer->blen];
        buffer->clen = 32;
    }
    buffer->current >>= 1;
    buffer->clen--;
    return value;
}
#endif