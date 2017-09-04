#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "constants.h"
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
uint32_t nio_get_bit(t_breader * reader, uint32_t * value);
uint32_t nio_get_bits(t_breader * reader, uint32_t * value, uint32_t len);
void io_backfeed(t_breader * reader, uint32_t buffer, uint32_t len);
void start_bwriter(t_bwriter * writer);
void nio_write_bit(uint32_t b, t_bwriter * writer);
void nio_write_bits(uint32_t val, uint32_t len, t_bwriter * writer);
void nio_flush_bits(t_bwriter * writer);
size_t get_file_size(FILE * stream);
int atend(t_breader * reader);
void start_iwriter(t_iwriter * writer);
void nio_write_int(uint32_t i, t_iwriter * writer);
void nio_flush_ints(t_iwriter * writer);
#endif