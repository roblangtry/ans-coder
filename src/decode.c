#include "decode.h"
int decode(char * input_filename, char * output_filename, unsigned char verbose_flag){
    struct header header;
    unsigned char flag_byte;
    unsigned char method;
    FILE * input_file;
    FILE * output_file;
    if(verbose_flag == 1){
        printf("Input filename:  %s\n", input_filename);
        printf("Output filename: %s\n", output_filename);
    }
    input_file = fopen(input_filename, "r");
    output_file = fopen(output_filename, "w");
    header = read_header(input_file, &flag_byte);
    if(verbose_flag == 1){
        printf("===========================\n");
        printf("  Header\n");
        printf("---------------------------\n");
        printf("no_symbols: %ld\n", (long)header.no_symbols);
        printf("no_unique_symbols: %ld\n", (long)header.no_unique_symbols);
        printf("===========================\n");
    }
    method = flag_byte % 2;
    if (method == 0){
        if(verbose_flag == 1)
            printf("rANS compression scheme\n");
        rANS_decode(input_file, output_file, header, verbose_flag);

    } else if (method == 1){
        if(verbose_flag == 1)
            printf("tANS compression scheme\n");
        printf("NOT IMPLEMENTED YET\n");
        return -1;
    }
    return 1;
}