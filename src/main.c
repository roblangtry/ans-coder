#include "main.h"
int main ( int argc, char *argv[] ){
    unsigned char decode_flag, encode_flag, method_flag, verbose_flag;
    int c=0;//, indent;
    char * input_filename;
    char * output_filename;
    decode_flag = 0;
    encode_flag = 0;
    method_flag = RANGE_METHOD;
    verbose_flag = 0;
    coding_signature_t signature = get_signature();
    while((c = getopt(argc, argv, "ba:detrvpxsVmM:Snk:K:P")) != -1){
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
                break;
            case 't':
                method_flag = TABLE_METHOD;
                break;
            case 'b':
                signature.header = HEADER_BLOCK;
                break;
            case 'x':
                method_flag = ESCAPE_METHOD;
                break;
            case 's':
                method_flag = SPLIT_METHOD;
                break;
            case 'V':
                method_flag = VECTOR_METHOD;
                break;
            case 'r':
                method_flag = 0;
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case 'm':
                signature.symbol = SYMBOL_MSB;
                break;
            case 'n':
                signature.symbol = SYMBOL_MSB_2;
                break;
            case 'M':
                signature.msb_bit_factor = atoi(optarg);
                break;
            case 'S':
                signature.translation = TRANSLATE_TRUE;
                break;
            case 'k':
                signature.translation = TRANSLATE_PARTIAL;
                signature.translate_k = atoi(optarg);
                break;
            case 'P':
                signature.translation = TRANSLATE_PERMUTATION_TRUE;
                break;
            case 'K':
                signature.translation = TRANSLATE_PERMUTATION_PARTIAL;
                signature.translate_k = atoi(optarg);
                break;
            case 'a':
                signature.bit_factor = atoi(optarg);
                break;
        }
    }
    input_filename = argv[argc - 2];
    output_filename = argv[argc - 1];
    if (decode_flag == 1){
        if(method_flag == RANGE_METHOD){
            FILE * input_file = fopen(input_filename, "r");
            FILE * output_file = fopen(output_filename, "w");
            return decode_file(input_file, output_file, signature);
        }
        else {
            return decode(input_filename, output_filename, verbose_flag);
        }
    }
    if (encode_flag == 1){
        if(method_flag == RANGE_METHOD){
            FILE * input_file = fopen(input_filename, "r");
            FILE * output_file = fopen(output_filename, "w");
            return encode_file(input_file, output_file, signature);
        }
        else {
            return encode(input_filename, output_filename, method_flag, verbose_flag);
        }
    }
}
