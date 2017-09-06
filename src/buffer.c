#include "buffer.h"

void
start_buffer(bit_buffer * buffer)
{
    buffer->current = 0;
    buffer->clen = 0;
    buffer->blen = 0;
}
void
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
void
set_buffer(uint32_t value, uint32_t len, bit_buffer * buffer)
{
    buffer->clen = len;
    buffer->current = value;
}
void
buffer_int(uint32_t value, bit_buffer * buffer)
{
    buffer->buffer[buffer->blen++] = value;
}
uint32_t get_buffered_bit(bit_buffer * buffer)
{
    uint32_t value;
    if(!buffer->clen){
        buffer->blen--;
        buffer->current = buffer->buffer[buffer->blen];
        buffer->clen = 32;
    }
    value = buffer->current & 1;
    buffer->current >>= 1;
    buffer->clen--;
    return value;
}