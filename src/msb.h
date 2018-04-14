#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "page.h"

#ifndef MSB_CODE
#define MSB_CODE

inline uint32_t get_msb_symbol(uint32_t symbol, uint32_t binary_length){
    uint32_t b=0,sym=symbol-1;
    while(sym>>=binary_length)
        b++;
    return (symbol>>(b*binary_length))+(b<<(binary_length));
}
inline uint32_t get_umsb_symbol(uint32_t symbol, uint32_t binary_length){
    return get_msb_symbol(symbol-1, binary_length);
}

inline uint32_t get_msb_2_symbol(uint32_t symbol, uint32_t binary_length){
    uint32_t add = 0;
    uint32_t offset = 0;
    uint64_t check = (1 << binary_length);
    if (symbol <= check)
    {
        return symbol;
    }
    while (symbol >= check)
    {
        offset = offset + 1;
        add = add + check;
        symbol = symbol >> 1;
    }
    return symbol + add;
}
inline uint32_t get_umsb_2_symbol(uint32_t symbol, uint32_t binary_length){

    return get_msb_2_symbol(symbol-1,binary_length);
}

inline uint32_t get_symbol(uint32_t input, coding_signature_t signature)
{
    if(signature.symbol == SYMBOL_DIRECT) return input;
    else if(signature.symbol == SYMBOL_MSB) return get_msb_symbol(input, signature.msb_bit_factor);
    else if(signature.symbol == SYMBOL_MSB_2) return get_msb_2_symbol(input, signature.msb_bit_factor);
    else exit(-1);
}
inline uint32_t get_usymbol(uint32_t input, coding_signature_t signature)
{
    if(signature.symbol == SYMBOL_DIRECT) return input;
    else if(signature.symbol == SYMBOL_MSB) return get_umsb_symbol(input, signature.msb_bit_factor);
    else if(signature.symbol == SYMBOL_MSB_2) return get_umsb_2_symbol(input, signature.msb_bit_factor);
    else exit(-1);
}


void stream_msb(uint32_t symbol, uint32_t bits, bint_page_t * pages);
void stream_msb_2(uint32_t symbol, uint32_t bits, bit_page_t * pages);
#endif