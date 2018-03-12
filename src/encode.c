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
    output_block_t block;
    block.pre = mymalloc(sizeof(uint32_t) * PRE_SIZE);
    block.pre_size = 0;
    block.pre_max_size = PRE_SIZE;
    block.content = mymalloc(sizeof(uint32_t) * CONTENT_SIZE);
    block.content_size = 0;
    block.content_max_size = CONTENT_SIZE;
    block.post = mymalloc(sizeof(uint32_t) * POST_SIZE);
    block.post_size = 0;
    block.post_max_size = POST_SIZE;
    file_header_t header;
    header.freq = mymalloc(sizeof(uint32_t) * SYMBOL_MAP_SIZE);
    header.cumalative_freq = mymalloc(sizeof(uint32_t) * SYMBOL_MAP_SIZE);
    header.data = mymalloc(sizeof(uint32_t) * BLOCK_SIZE);
    header.max = 0;
    header.symbols = 0;
    header.unique_symbols = 0;
    struct writer * my_writer = initialise_writer(output_file);
    preprocess_file(input_file, signature, &header);
    output_file_header(my_writer, &header, signature);
    for(int i = 0; i < header.no_blocks; i++)
    {
        process_block(input_file, &header, signature, &block);
        output_block(my_writer, &block);
    }
    flush_writer(my_writer);
    printmem();
    return 1;
}