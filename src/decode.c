#include "decode.h"
int decode(char * input_filename, char * output_filename, unsigned char verbose_flag){
    struct header header;
    unsigned char flag_byte;
    unsigned char method;
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
    if(!fread(&flag_byte, sizeof(unsigned char), 1, input_file)) flag_byte = 0;
    if(flag_byte < 2){
        header = read_header(input_file, &flag_byte);
        if(verbose_flag == 1){
            fprintf(stderr, "===========================\n");
            fprintf(stderr, "  Header\n");
            fprintf(stderr, "---------------------------\n");
            fprintf(stderr, "no_symbols: %ld\n", (long)header.no_symbols);
            fprintf(stderr, "no_unique_symbols: %ld\n", (long)header.no_unique_symbols);
            fprintf(stderr, "===========================\n");
        }
        method = flag_byte % 4;
        if (method == 0){
            if(verbose_flag == 1)
                fprintf(stderr, "rANS compression scheme\n");
            rANS_decode(input_file, output_file, header, verbose_flag);
    
        } else if (method == 1){
            if(verbose_flag == 1)
                fprintf(stderr, "tANS compression scheme\n");
            /* tANS_decode(input_file, output_file, header, verbose_flag); */
            return -1;
        }
    } else{
        method = flag_byte;
        // if(method == VECTOR_METHOD){
        //     if(verbose_flag == 1) fprintf(stderr, "vANS compression scheme\n");
        //     vANS_decode(input_file, output_file, my_prelude_functions);
        // }
        if(method == ESCAPE_METHOD){
            if(verbose_flag == 1) fprintf(stderr, "xANS compression scheme\n");
            bANS_decode(input_file, output_file, my_prelude_functions, method);
        }
        else if(method == SPLIT_METHOD){
            if(verbose_flag == 1) fprintf(stderr, "sANS compression scheme\n");
            bANS_decode(input_file, output_file, my_prelude_functions, method);
        }
        else{
            if(verbose_flag == 1) fprintf(stderr, "bANS compression scheme\n");
            bANS_decode(input_file, output_file, my_prelude_functions, method);
        }
        printmem();
        return -1;
    }
    printmem();
    return 1;
}


int decode_file(FILE * input_file, FILE * output_file, coding_signature_t signature)
{
    file_header_t * header = mymalloc(sizeof(file_header_t));
    header->freq = mymalloc(sizeof(uint32_t) * SYMBOL_MAP_SIZE);
    header->cumalative_freq = mymalloc(sizeof(uint32_t) * SYMBOL_MAP_SIZE);
    header->data = mymalloc(sizeof(uint32_t) * BLOCK_SIZE);
    header->max = 0;
    header->symbols = 0;
    header->unique_symbols = 0;
    data_block_t * data = mymalloc(sizeof(data_block_t));
    data->data = mymalloc(sizeof(uint32_t) * BLOCK_SIZE);
    read_file_header(input_file, &signature, header);
    for(int i = 0; i < header->no_blocks; i++)
    {
        read_block(input_file, header, signature, data);
        output_to_file(output_file, data);
    }
    myfree(data->data);
    myfree(data);
    myfree(header->cumalative_freq);
    myfree(header->freq);
    myfree(header->data);
    myfree(header);
    printmem();
    return 1;
}