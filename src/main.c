#include "main.h"
int main ( int argc, char *argv[] ){
    unsigned char decode_flag, encode_flag, method_flag, verbose_flag;
    int c;//, indent;
    char * input_filename;
    char * output_filename;
    decode_flag = 0;
    encode_flag = 0;
    method_flag = RANGE_METHOD;
    verbose_flag = 0;
    // indent = 0;
    coding_signature_t signature = get_signature();
    while((c = getopt(argc, argv, "bdetrvpxsVm")) != -1){
        switch(c)
        {
            case 'd':
                decode_flag = 1;
                break;
            case 'e':
                encode_flag = 1;
                break;
            case 'p':
                method_flag = PARRALEL_METHOD;
                // indent = 1;
                break;
            case 't':
                method_flag = TABLE_METHOD;
                break;
            case 'b':
                method_flag = BLOCK_METHOD;
                // indent = 1;
                break;
            case 'x':
                method_flag = ESCAPE_METHOD;
                // indent = 1;
                break;
            case 's':
                method_flag = SPLIT_METHOD;
                // indent = 1;
                break;
            case 'V':
                method_flag = VECTOR_METHOD;
                // indent = 1;
                break;
            case 'r':
                method_flag = 0;
                // indent = 1;
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case 'm':
                //method_flag = MSB_METHOD;
                signature.symbol = SYMBOL_MSB;
                break;
        }
    }
    // if(( (decode_flag + encode_flag) != 1  || (2 + verbose_flag + indent + 2) != argc) && !(method_flag == PARRALEL_METHOD || method_flag == TABLE_METHOD)){
    //     fprintf(stderr, "CORRECT SYNTAX:\n");
    //     fprintf(stderr, "    %s -e [-r | -t] [-v] <input_file> <output_file>\n", argv[0]);
    //     fprintf(stderr, " or\n");
    //     fprintf(stderr, "    %s -d [-v] <input_file> <output_file> \n", argv[0]);
    //     return -1;
    // }
    input_filename = argv[argc - 2];
    output_filename = argv[argc - 1];
    if (decode_flag == 1){
        if(method_flag == PARRALEL_METHOD){
            return pans_decode();
        }
        else if(method_flag == TABLE_METHOD){
            return tans_decode();
        }
        else if(method_flag == RANGE_METHOD){
            FILE * input_file = fopen(input_filename, "r");
            FILE * output_file = fopen(output_filename, "w");
            return decode_file(input_file, output_file, signature);
        }
        else {
            return decode(input_filename, output_filename, verbose_flag);
        }
    }
    if (encode_flag == 1){
        if(method_flag == PARRALEL_METHOD){
            return pans_encode();
        }
        else if(method_flag == TABLE_METHOD){
            return tans_encode();
        }
        else if(method_flag == RANGE_METHOD){
            FILE * input_file = fopen(input_filename, "r");
            FILE * output_file = fopen(output_filename, "w");
            return encode_file(input_file, output_file, signature);
        }
        else {
            return encode(input_filename, output_filename, method_flag, verbose_flag);
        }
    }
}
