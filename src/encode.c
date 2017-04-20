#include "encode.h"
int encode(char * input_filename, char * output_filename, unsigned char method_flag, unsigned char verbose_flag){
    struct header header;
    unsigned char flag_byte;
    int i;
    FILE * input_file;
    FILE * output_file;
    if(verbose_flag == 1){
        printf("Input filename:  %s\n", input_filename);
        printf("Output filename: %s\n", output_filename);
    }
    input_file = fopen(input_filename, "r");
    output_file = fopen(output_filename, "w");
    header = preprocess(input_file);
    flag_byte = method_flag;
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
    }
    if (method_flag == 1){
        if(verbose_flag == 1)
            printf("tANS compression scheme\n");
        printf("NOT IMPLEMENTED YET\n");
        return -1;
    }
    if (method_flag == 0){
        if(verbose_flag == 1)
            printf("rANS compression scheme\n");
        rANS_encode(input_file, output_file, header);
    }
    return 1;
}