#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "mem_manager.h"
#include "file_header.h"


#ifndef MAP_CODE
#define MAP_CODE
typedef struct
{
    uint32_t key;
    uint32_t value;
} kv_t;
void ADD(uint32_t key,uint32_t value);
uint32_t INCREMENT(uint32_t key);
uint32_t GET(uint32_t key);
kv_t UGET(uint32_t ukey);
void SETUP(size_t map_size, size_t sentinal);
void CLEAR();
#endif