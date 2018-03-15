#include "msb.h"

uint32_t get_msb_symbol(uint32_t symbol){
    if(symbol <= 256) return symbol;
    else if(symbol <= 65536) return (symbol >> 8) + 256;
    else if(symbol <= 16777216) return (symbol >> 16) + 512;
    fprintf(stderr, "ERROR: MSB number issue\n");
    exit(-1);
}
uint32_t get_msb_rounds(uint32_t symbol){
    if(symbol <= 256) return 0;
    else if(symbol <= 65536) return 1;
    else if(symbol <= 16777216) return 2;
    fprintf(stderr, "ERROR: MSB number issue\n");
    exit(-1);
}
