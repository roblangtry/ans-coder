#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "mem_manager.h"
#include "constants.h"
#ifndef SPARSE_HASH_CODE
#define SPARSE_HASH_CODE
typedef struct
{
    uint32_t max;
    uint32_t ** values;
} sparse_hash_t;
sparse_hash_t * sparse_hash_create(uint32_t max);
uint32_t sparse_hash_get(uint32_t key, sparse_hash_t * hash);
uint32_t sparse_hash_set(uint32_t key, uint32_t value, sparse_hash_t * hash);
uint32_t sparse_hash_increment(uint32_t key, uint32_t value, sparse_hash_t * hash);
void sparse_hash_free(sparse_hash_t * hash);

#endif