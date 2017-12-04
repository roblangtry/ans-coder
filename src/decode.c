#include "decode.h"
int decode(char * input_filename, char * output_filename, unsigned char verbose_flag){
    struct header header;
    unsigned char flag_byte;
    unsigned char method;
    FILE * input_file;
    FILE * output_file;
    struct prelude_functions * my_prelude_functions = malloc(sizeof(struct prelude_functions));
    my_prelude_functions->func_encode = vbyte_encode;
    my_prelude_functions->func_flush = vbyte_flush;
    my_prelude_functions->func_decode = vbyte_decode;
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
        method = flag_byte % 2;
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
        if(verbose_flag == 1)
            fprintf(stderr, "bANS compression scheme\n");
        if(method == VECTOR_METHOD)
            vANS_decode(input_file, output_file, my_prelude_functions);
        else
            bANS_decode(input_file, output_file, my_prelude_functions);
        return -1;
    }
    return 1;
}