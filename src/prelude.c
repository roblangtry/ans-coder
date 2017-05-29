#include "prelude.h"
void write_symbol_prelude(uint32_t * symbols, uint32_t * symbol_frequencies, size_t * no_symbols, FILE * output_file, uint64_t * state, size_t * content_length)
{
    fwrite(state, sizeof(uint64_t), 1, output_file);
    fwrite(no_symbols, sizeof(size_t), 1, output_file);
    fwrite(content_length, sizeof(size_t), 1, output_file);
    struct bitlevel_file_pointer * bfp = get_bitlevel_file_pointer(output_file);
    uint64_t i = 0;
    uint64_t last_symbol = 0;
    while(i < *no_symbols){
        write_elias_value(bfp, symbols[i] - last_symbol);
        last_symbol = symbols[i];
        write_elias_value(bfp, symbol_frequencies[i]);
        i++;
    }
    bitlevel_flush(bfp);
}
void read_symbol_prelude(size_t * no_symbols, FILE * input_file, uint32_t ** symbols, uint32_t ** symbol_frequencies, uint64_t * state, size_t * content_length)
{
    struct bitlevel_file_pointer * bfp = get_unbuffered_bitlevel_file_pointer(input_file);
    uint64_t i = 0;
    uint64_t last_symbol = 0;
    fread(state, sizeof(uint64_t), 1, input_file);
    fread(no_symbols, sizeof(size_t), 1, input_file);
    fread(content_length, sizeof(size_t), 1, input_file);
    *symbols = malloc(sizeof(uint32_t) * (*no_symbols));
    *symbol_frequencies = malloc(sizeof(uint32_t) * (*no_symbols));
    while(i < *no_symbols){
        (*symbols)[i] = (uint32_t)read_elias_value(bfp) + last_symbol;
        last_symbol = (*symbols)[i];
        (*symbol_frequencies)[i] = (uint32_t)read_elias_value(bfp);
        i++;
    }
}