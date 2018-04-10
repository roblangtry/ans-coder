#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "mem_manager.h"
#include "file_header.h"
#include "map.h"

#ifndef ORDER_CODE
#define ORDER_CODE
typedef struct
{
    uint32_t freq;
    uint32_t index;
} tuple_t;
void build_translations_decoding(file_header_t * header,coding_signature_t signature, struct prelude_code_data * metadata);
void build_translations_encoding(file_header_t * header, uint32_t size, struct prelude_code_data * metadata,coding_signature_t signature);
tuple_t * get_tuples(uint32_t * freq, uint32_t no_unique);
uint32_t * get_translation_matrix(tuple_t * tuples, uint32_t length, uint32_t max, file_header_t * header,coding_signature_t signature, struct prelude_code_data * metadata);
uint32_t * get_reverse_translation_matrix(tuple_t * tuples, uint32_t length, file_header_t * header, struct prelude_code_data * metadata);
void ksort(tuple_t * tuples, uint32_t length, uint32_t k);
void kcheck(size_t i, size_t * top, uint32_t k, tuple_t * tuples);
inline int translating(uint32_t flag){
    return flag == TRANSLATE_TRUE || flag == TRANSLATE_PARTIAL || flag == TRANSLATE_PERMUTATION_TRUE || flag == TRANSLATE_PERMUTATION_PARTIAL;
}
inline int full_translating(uint32_t flag){
    return flag == TRANSLATE_TRUE || flag == TRANSLATE_PARTIAL;
}
inline int perm_translating(uint32_t flag){
    return flag == TRANSLATE_PERMUTATION_TRUE || flag == TRANSLATE_PERMUTATION_PARTIAL;
}
inline int partial_translating(uint32_t flag){
    return flag == TRANSLATE_PARTIAL || flag == TRANSLATE_PERMUTATION_PARTIAL;
}
#endif