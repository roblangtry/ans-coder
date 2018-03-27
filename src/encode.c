#include "encode.h"
int encode(char * input_filename, char * output_filename, unsigned char method_flag, unsigned char verbose_flag){
    struct header header;
    unsigned char flag_byte;
    int i;
    FILE * input_file;
    FILE * output_file;
    struct prelude_functions * my_prelude_functions = mymalloc(sizeof(struct prelude_functions));
    my_prelude_functions->func_encode = elias_encode;
    my_prelude_functions->func_flush = elias_flush;
    my_prelude_functions->func_decode = elias_decode;
    if(verbose_flag == 1){
        fprintf(stderr, "Input filename:  %s\n", input_filename);
        fprintf(stderr, "Output filename: %s\n", output_filename);
    }
    input_file = fopen(input_filename, "r");
    output_file = fopen(output_filename, "w");
    flag_byte = method_flag;
    if (method_flag < 2){
        header = preprocess(input_file);
        writeout_header(output_file, header, flag_byte);
        if(verbose_flag == 1){
            fprintf(stderr, "===========================\n");
            fprintf(stderr, "  Header\n");
            fprintf(stderr, "---------------------------\n");
            fprintf(stderr, "no_symbols: %ld\n", (long)header.no_symbols);
            fprintf(stderr, "no_unique_symbols: %ld\n", (long)header.no_unique_symbols);
            fprintf(stderr, "===========================\n");
            i = 0;
            while(i < header.no_unique_symbols){
                fprintf(stderr, "SYM %ld | FRQ %ld\n",
                (long)header.symbols[i],
                (long)header.symbol_frequencies[i]);
                i++;
            }
            fprintf(stderr, "===========================\n");
        }
        if (method_flag == 1){
            if(verbose_flag == 1)
                fprintf(stderr, "tANS compression scheme\n");
            /* tANS_encode(input_file, output_file, header); */
            return 1;
        }
        if (method_flag == 0){
            if(verbose_flag == 1)
                fprintf(stderr, "rANS compression scheme\n");
            rANS_encode(input_file, output_file, header);
        }

    }
    else {
        fwrite(&flag_byte, sizeof(unsigned char), 1, output_file);
        if(verbose_flag == 1)
            fprintf(stderr, "bANS compression scheme\n");
        // if(method_flag == VECTOR_METHOD)
        //     vANS_encode(input_file, output_file, my_prelude_functions);
        if(method_flag == ESCAPE_METHOD)
            bANS_encode(input_file, output_file, my_prelude_functions, method_flag);
        else if(method_flag == SPLIT_METHOD)
            bANS_encode(input_file, output_file, my_prelude_functions, method_flag);
        else
            bANS_encode(input_file, output_file, my_prelude_functions, method_flag);

    }

    printmem();
    return 1;
}

int encode_file(FILE * input_file, FILE * output_file, coding_signature_t signature)
{
    file_header_t header;
    uint64_t sym_map_size = BLOCK_SIZE+1;
    uint32_t msb_bits = signature.msb_bit_factor;
    if(signature.symbol == SYMBOL_MSB) sym_map_size = get_msb_symbol(SYMBOL_MAP_SIZE, msb_bits);
    if(signature.symbol == SYMBOL_MSB_2) sym_map_size = get_msb_2_symbol(SYMBOL_MAP_SIZE, msb_bits);
    if(signature.hashing == HASHING_STANDARD) header.freq = calloc(sizeof(uint32_t), sym_map_size);
    header.data = mymalloc(sizeof(uint32_t) * BLOCK_SIZE);
    header.max = 0;
    header.global_max = 0;
    header.symbols = 0;
    header.unique_symbols = 0;
    struct writer * my_writer = initialise_writer(output_file);
    preprocess_file(input_file, signature, &header);
    output_file_header(my_writer, &header, signature);
    for(int i = 0; i < header.no_blocks; i++)
    {
        process_block(input_file, my_writer, &header, signature);
    }
    flush_writer(my_writer);
    myfree(my_writer);
    myfree(header.freq);
    myfree(header.data);
    printmem();
    return 1;
}