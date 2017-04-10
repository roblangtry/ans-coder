#include "encode.h"
int encode(char * input_filename, char * output_filename, unsigned char method_flag, unsigned char verbose_flag){
    struct header header;
    FILE * input_file;
    FILE * output_file;
    if(verbose_flag == 1){
        printf("Input filename:  %s\n", input_filename);
        printf("Output filename: %s\n", output_filename);
    }
    input_file = fopen(input_filename, "r");
    output_file = fopen(output_filename, "w");
    header = preprocess(input_file);
    if(verbose_flag == 1){
        printf("===========================\n");
        printf("  Header\n");
        printf("---------------------------\n");
        printf("no_symbols: %ld\n", (long)header.no_symbols);
        printf("no_unique_symbols: %ld\n", (long)header.no_unique_symbols);
        printf("===========================\n");
    }
    if (method_flag == 2){
        if(verbose_flag == 1)
            printf("tANS compression scheme\n");
        printf("NOT IMPLEMENTED YET\n");
        return -1;
    }
    if (method_flag == 1){
        if(verbose_flag == 1)
            printf("rANS compression scheme\n");
    }
    return 1;
}