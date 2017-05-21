#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "elias.h"
void write_symbol_prelude(uint32_t * symbols, uint32_t * symbol_frequencies, size_t no_symbols, FILE * output_file);
void read_symbol_prelude(size_t no_symbols, FILE * input_file, uint32_t ** symbols, uint32_t ** symbol_frequencies);