#include "coders.h"
uint32_t first = 0;
uint32_t logs[SYMBOL_MAP_SIZE];
void
binary_encode(uint32_t value, uint32_t length, t_bwriter * writer)
{
    nio_write_bits(value, length, writer);
}
uint32_t
binary_decode(uint32_t * V,uint32_t length, t_breader * reader)
{
    int i = nio_get_bits(reader, V, length);
        return i;
}
void
unary_encode(uint32_t value, t_bwriter * writer)
{
    uint32_t v = value, x = 0, add = 0;

    while(v > 1)
    {
        x = (x << 1) + 1;
        add++;
        if(add==30){
            nio_write_bits(x, add, writer);
            add = 0;
            x = 0;
        }
        v--;
    }
    nio_write_bits(x<<1, add+1, writer);
}
uint32_t
unary_decode(uint32_t * V, t_breader * reader)
{
    uint32_t b;
    *V = 1;
    if (nio_get_bit(reader, &b) == 0) return 0;
    while(b == 1)
    {
        *V= *V + 1;
        if (nio_get_bit(reader, &b) == 0) return 0;
    }
    return 1;
}
void
elias_gamma_encode(uint32_t value, t_bwriter * writer)
{
    uint32_t l = mylog(value);
    unary_encode(l+1, writer);
    binary_encode(value, l, writer);
}
uint32_t
elias_gamma_decode(uint32_t * V,t_breader * reader)
{
    uint32_t l;
    if(unary_decode(&l, reader) == 0) return 0;
    l--;
    if(l == 0){
        *V = 1;
        return 1;
    }
    if(binary_decode(V, l, reader) == 0) return 0;
    /* *V = *V + (1 << l); */
    return 1;
}
void
elias_delta_encode(uint32_t value, t_bwriter * writer)
{
    uint32_t l = mylog(value);
    elias_gamma_encode(l+1, writer);
    binary_encode(value, l, writer);
}
uint32_t
elias_delta_decode(uint32_t * V,t_breader * reader)
{
    uint32_t l;
    if(elias_gamma_decode(&l, reader) == 0) return 0;
    l--;
    if(l == 0){
        *V = 1;
        return 1;
    }
    if(binary_decode(V, l, reader) == 0) return 0;
    /* *V = *V + (1 << l); */
    return 1;
}


uint32_t mylog(uint32_t value){
    if(first == 0){
        uint32_t index = 1;
        uint32_t length = 0;
        uint32_t width = 0;
        while(index < SYMBOL_MAP_SIZE)
        {
            if(width == 0){
                width = 1 << length;
                length++;
            }
            logs[index++] = length;
            width--;
        }
        first = 1;
    }
    if(value > SYMBOL_MAP_SIZE) return flylog(value);
    return logs[value];
}

uint32_t flylog( uint32_t x )
{
  uint32_t ans = 0;
  while(x>>=1) ans++;
  return ans;
}