#include "encode.h"
int encode(char * input_filename, char * output_filename, unsigned char method_flag, unsigned char verbose_flag){
    struct header header;
    unsigned char flag_byte;
    int i;
    FILE * input_file;
    FILE * output_file;
    struct prelude_functions * my_prelude_functions = malloc(sizeof(struct prelude_functions));
    my_prelude_functions->func_encode = vbyte_encode;
    my_prelude_functions->func_flush = vbyte_flush;
    my_prelude_functions->func_decode = vbyte_decode;
    if(verbose_flag == 1){
        printf("Input filename:  %s\n", input_filename);
        printf("Output filename: %s\n", output_filename);
    }
    input_file = fopen(input_filename, "r");
    output_file = fopen(output_filename, "w");
    flag_byte = method_flag;
    if (method_flag < 2){
        header = preprocess(input_file);
        writeout_header(output_file, header, flag_byte);
        if(verbose_flag == 1){
            printf("===========================\n");
            printf("  Header\n");
            printf("---------------------------\n");
            printf("no_symbols: %ld\n", (long)header.no_symbols);
            printf("no_unique_symbols: %ld\n", (long)header.no_unique_symbols);
            printf("===========================\n");
            i = 0;
            while(i < header.no_unique_symbols){
                printf("SYM %ld | FRQ %ld\n",
                (long)header.symbols[i],
                (long)header.symbol_frequencies[i]);
                i++;
            }
            printf("===========================\n");
        }
        if (method_flag == 1){
            if(verbose_flag == 1)
                printf("tANS compression scheme\n");
            /* tANS_encode(input_file, output_file, header); */
            return 1;
        }
        if (method_flag == 0){
            if(verbose_flag == 1)
                printf("rANS compression scheme\n");
            rANS_encode(input_file, output_file, header);
        }

    }
    else {
        fwrite(&flag_byte, sizeof(unsigned char), 1, output_file);
        if(verbose_flag == 1)
            printf("bANS compression scheme\n");
        bANS_encode(input_file, output_file, my_prelude_functions);

    }

    return 1;
}