#include "prelude.h"
void write_symbol_prelude(uint32_t * symbols, uint32_t * symbol_frequencies, size_t * no_symbols, uint64_t * state, size_t * content_length, struct writer * my_writer, struct prelude_functions * my_prelude_functions)
{
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    my_prelude_functions->func_encode(metadata, *state);
    my_prelude_functions->func_encode(metadata, *no_symbols);
    my_prelude_functions->func_encode(metadata, *content_length);
    //printf("state (%ld)\n", *state);
    //printf("no_symbols (%ld)\n", *no_symbols);
    //printf("content_length (%ld)\n", *content_length);
    uint64_t i = 0;
    uint64_t last_symbol = 0;
    while(i < *no_symbols){
        my_prelude_functions->func_encode(metadata, symbols[i] - last_symbol);
        last_symbol = symbols[i];
        my_prelude_functions->func_encode(metadata, symbol_frequencies[i]);
        i++;
    }
    my_prelude_functions->func_flush(metadata);
    free_metadata(metadata);
}
void read_symbol_prelude(size_t * no_symbols, uint32_t ** symbols, uint32_t ** symbol_frequencies, uint64_t * state, size_t * content_length, struct reader * my_reader, struct prelude_functions * my_prelude_functions)
{
    uint64_t i = 0;
    uint64_t last_symbol = 0;
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    *state = my_prelude_functions->func_decode(metadata);
    *no_symbols = my_prelude_functions->func_decode(metadata);
    *content_length = my_prelude_functions->func_decode(metadata);
    //printf("state (%ld)\n", *state);
    //printf("no_symbols (%ld)\n", *no_symbols);
    //printf("content_length (%ld)\n", *content_length);
    *symbols = mymalloc(sizeof(uint32_t) * (*no_symbols));
    *symbol_frequencies = mymalloc(sizeof(uint32_t) * (*no_symbols));
    while(i < *no_symbols){
        (*symbols)[i] = my_prelude_functions->func_decode(metadata) + last_symbol;
        last_symbol = (*symbols)[i];
        (*symbol_frequencies)[i] = my_prelude_functions->func_decode(metadata);
        i++;
    }
    free_metadata(metadata);
}
