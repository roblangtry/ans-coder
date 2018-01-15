#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "nio.h"
#include "constants.h"
#ifndef CODERS_CODE
#define CODERS_CODE

static inline uint32_t flylog( uint32_t x )
{
  uint32_t ans = 1;
  while(x>>=1) ans++;
  return ans;
}
static inline void
binary_encode(uint32_t value, uint32_t length, t_bwriter * writer)
{
    nio_write_bits(value, length, writer);
}
static inline uint32_t
binary_decode(uint32_t * V,uint32_t length, t_breader * reader)
{
    int i = nio_get_bits(reader, V, length);
        return i;
}
static inline void
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
static inline uint32_t
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
static inline void
elias_gamma_encode(uint32_t value, t_bwriter * writer)
{
    uint32_t l = flylog(value);
    unary_encode(l+1, writer);
    binary_encode(value, l, writer);
}
static inline uint32_t
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
static inline void
elias_delta_encode(uint32_t value, t_bwriter * writer)
{
    uint32_t l = flylog(value);
    elias_gamma_encode(l+1, writer);
    binary_encode(value, l, writer);
}
static inline uint32_t
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



#endif