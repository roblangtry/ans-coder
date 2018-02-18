#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "elias.h"
#include "prelude_code.h"
#include "mem_manager.h"
#ifndef PRELUDE_CODE
#define PRELUDE_CODE
void write_symbol_prelude(uint32_t * symbols, uint32_t * symbol_frequencies, size_t * no_symbols, uint64_t * state, size_t * content_length, struct writer * my_writer, struct prelude_functions * my_prelude_functions);
void read_symbol_prelude(size_t * no_symbols, uint32_t ** symbols, uint32_t ** symbol_frequencies, uint64_t * state, size_t * content_length, struct reader * my_reader, struct prelude_functions * my_prelude_functions);
#endif