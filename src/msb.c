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
