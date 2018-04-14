#include "msb.h"

void stream_msb(uint32_t symbol, uint32_t bits, bint_page_t * pages)
{
    uint32_t byte;
    uint32_t j = 0;
    uint32_t v = symbol - 1;
    while ((v >> bits) > 0){
        j = j + 1;
        v = v >> bits;
    }
    while(j > 0)
    {
        if (j == 1)
            byte = symbol % (1 << bits);
        else{
            byte = (symbol >> (bits * (j-1))) % (1 << bits);
        }
        add_to_bint_page(byte, bits, pages);
        j = j - 1;
    }
}

void stream_msb_2(uint32_t symbol, uint32_t bits, bit_page_t * pages)
{
    uint32_t j = 0;
    uint32_t sym = symbol;
    if(sym > (1 << bits))
    {
        while(sym>=(1 << bits))
        {
            sym = sym >> 1;
            j++;
        }
    }
    if(j > 0){
        add_to_bit_page(symbol % (1<<j), j, pages);
    }
}