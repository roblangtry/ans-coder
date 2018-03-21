#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "mem_manager.h"
#include "file_header.h"

#ifndef ORDER_CODE
#define ORDER_CODE
typedef struct
{
    uint32_t freq;
    uint32_t index;
} tuple_t;
void build_translations_encoding(file_header_t * header, uint32_t size, struct prelude_code_data * metadata);
tuple_t * get_tuples(uint32_t * freq, uint32_t no_unique);
uint32_t * get_translation_matrix(tuple_t * tuples, uint32_t length, uint32_t max);
uint32_t * get_reverse_translation_matrix(tuple_t * tuples, uint32_t length, uint32_t max);
#endif