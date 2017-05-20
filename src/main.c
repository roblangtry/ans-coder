#include "main.h"
int main ( int argc, char *argv[] ){
    unsigned char decode_flag, encode_flag, method_flag, verbose_flag;
    int c, indent;
    char * input_filename;
    char * output_filename;
    decode_flag = 0;
    encode_flag = 0;
    method_flag = 0;
    verbose_flag = 0;
    indent = 0;
    while((c = getopt(argc, argv, "bdetrv")) != -1){
        switch(c)
        {
            case 'd':
                decode_flag = 1;
                break;
            case 'e':
                encode_flag = 1;
                break;
            case 't':
                method_flag = 1;
                indent = 1;
                break;
            case 'b':
                method_flag = 2;
                indent = 1;
                break;
            case 'r':
                method_flag = 0;
                indent = 1;
                break;
            case 'v':
                verbose_flag = 1;
                break;
        }
    }
    if( (decode_flag + encode_flag) != 1  || (2 + verbose_flag + indent + 2) != argc){
        printf("CORRECT SYNTAX:\n");
        printf("    %s -e [-r | -t] [-v] <input_file> <output_file>\n", argv[0]);
        printf(" or\n");
        printf("    %s -d [-v] <input_file> <output_file> \n", argv[0]);
        return -1;
    }
    input_filename = argv[argc - 2];
    output_filename = argv[argc - 1];
    if (decode_flag == 1)
        return decode(input_filename, output_filename, verbose_flag);
    if (encode_flag == 1)
        return encode(input_filename, output_filename, method_flag, verbose_flag);
}
