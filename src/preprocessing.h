#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "elias.h"
#include "constants.h"
#include "mem_manager.h"
#ifndef PREPROCESSING_CODE
#define PREPROCESSING_CODE
#define HASHMAP_SIZE 1048576
#define BUFFER_SIZE 1048576
struct header {
    unsigned char coding; //0 encode 1 decode
    uint64_t no_symbols;
    uint64_t no_unique_symbols;
    uint64_t max_symbol;
    uint64_t * hashmap;
    uint64_t * symbols;
    uint64_t * symbol_frequencies;
};
struct hashmap_node {
    uint64_t symbol;
    uint64_t index;
    struct hashmap_node * next;
};
struct header preprocess(FILE * input_file);
void ** initialise_hashmap();
// uint64_t get_symbol_index(uint64_t symbol, struct header * header);
uint64_t safe_get_symbol_index(uint64_t symbol, struct header * header);
struct hashmap_node * add_symbol(uint64_t symbol, struct header * header);
void check_for_rearrangement(struct hashmap_node * current, struct hashmap_node * last, struct header * header);
void writeout_header(FILE * output_file, struct header header, unsigned char flag_byte);
struct header read_header(FILE * input_file, unsigned char * flag_byte);
#endif