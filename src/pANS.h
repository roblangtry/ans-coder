#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <omp.h>
#include "coders.h"
#include "nio.h"
#include "buffer.h"
#ifndef PANS_CODE
#define PANS_CODE
#define THREADS 32
#define MIN(a,b) (((a)<(b))?(a):(b))
typedef struct {
    uint32_t len;
    uint32_t val;
} io_back;
typedef struct {
    uint32_t content[BLOCK_SIZE];
    size_t len;
} C_block_t;
int pans_encode();
int pans_decode();
void parralel_encode_block(C_block_t * block, t_bwriter * writer);
uint32_t parralel_decode_block(t_breader * reader, t_iwriter * writer, io_back * backfeed);
#endif