#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "constants.h"
#include "mem_manager.h"
#ifndef NIO_CODE
#define NIO_CODE
typedef struct {
    uint32_t buffer[BUFFER_SZ];
    uint64_t current;
    int head;
    size_t buffer_head;
} t_bwriter;
typedef struct {
    uint32_t buffer[BUFFER_SZ];
    size_t head;
} t_iwriter;
typedef struct {
    uint32_t buffer[BUFFER_SZ];
    uint64_t current;
    size_t length;
    int head;
    size_t buffer_head;
    int end;
} t_breader;
void start_breader(t_breader * reader);
static inline uint32_t nio_get_bits(t_breader * reader, uint32_t * value, uint32_t len)
{
    uint32_t lint, rint;
    if(len > reader->head){
        /* pull from buffer */
        if(reader->buffer_head == reader->length)
        {
            if(reader->end) return 0;
            reader->length = fread(reader->buffer, sizeof(uint32_t), BUFFER_SZ, stdin);
            if(reader->length != BUFFER_SZ) reader->end = 1;
            reader->buffer_head = 0;
        }
        if(reader->buffer_head == reader->length) return 0;
        else
        {
            lint = reader->current << (len - reader->head);
            reader->head = 32 - (len - reader->head);
            rint = reader->buffer[reader->buffer_head] >> reader->head;
            reader->current = reader->buffer[reader->buffer_head++] % (1 << reader->head);
            *value = lint + rint;
        }
    }
    else
    {
        *value = reader->current >> (reader->head - len);
        reader->current = reader->current - (*value << (reader->head - len));
        reader->head = reader->head - len;
    }
    return 1;
}
static inline uint32_t nio_get_bit(t_breader * reader, uint32_t * value){
    return nio_get_bits(reader, value, 1);
}
void io_backfeed(t_breader * reader, uint32_t buffer, uint32_t len);
void start_bwriter(t_bwriter * writer);
static inline void nio_write_bits(uint32_t val, uint32_t len, t_bwriter * writer){
    uint32_t bits_from_lint;
    uint32_t bits_from_rint;
    uint32_t lint;
    uint32_t rint;
    if(writer->head + len >= 32)
    {
        bits_from_lint = writer->head;
        bits_from_rint = 32 - bits_from_lint;
        lint = writer->current << bits_from_rint;
        rint = val >> (len - bits_from_rint);
        writer->buffer[writer->buffer_head++] = lint + rint;
        writer->current = val - (rint << (len - bits_from_rint));
        writer->head = len - bits_from_rint;
        if(writer->buffer_head == BUFFER_SZ)
        {
            fwrite(writer->buffer, sizeof(uint32_t), writer->buffer_head, stdout);
            writer->buffer_head = 0;
        }
    }
    else
    {
        writer->current = (writer->current << len) + val;
        writer->head += len;
    }
}
static inline void nio_write_bit(uint32_t b, t_bwriter * writer){
    nio_write_bits(b, 1, writer);
}
void nio_flush_bits(t_bwriter * writer);
size_t get_file_size(FILE * stream);
int atend(t_breader * reader);
void start_iwriter(t_iwriter * writer);
void nio_write_int(uint32_t i, t_iwriter * writer);
void nio_flush_ints(t_iwriter * writer);
#endif