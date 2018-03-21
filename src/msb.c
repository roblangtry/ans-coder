#include "msb.h"

uint32_t get_msb_symbol(uint32_t symbol, uint32_t binary_length){
    uint32_t add = 0;
    uint32_t offset = 0;
    uint64_t check = (1 << binary_length);
    if (symbol <= (1<<binary_length))
    {
        return symbol;
    }
    while (symbol > check)
    {
        offset = offset + binary_length;
        add = add + (1 << binary_length);
        check = check << binary_length;
    }
    return (symbol >> offset) + add;
}
uint32_t get_umsb_symbol(uint32_t symbol, uint32_t binary_length){
    uint32_t add = 0;
    uint32_t offset = 0;
    uint64_t check = (1 << binary_length);
    if (symbol < (1<<binary_length))
    {
        return symbol;
    }
    while (symbol >= check)
    {
        offset = offset + binary_length;
        add = add + (1 << binary_length);
        check = check << binary_length;
    }
    return (symbol >> offset) + add - 1;
}
void stream_msb(uint32_t symbol, uint32_t bits, int_page_t * pages)
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
        else
            byte = (symbol >> (bits * (j-1))) % (1 << bits);
        add_to_int_page(byte, pages);
        j = j - 1;
    }
}