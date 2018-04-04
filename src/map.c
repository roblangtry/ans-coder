#include "map.h"
uint32_t * MAP = NULL;
kv_t * DENSE = NULL;
size_t SIZE = 0;
size_t SENTINEL = 0;
size_t PAGES = 0;
void ADD(uint32_t key,uint32_t value)
{
    MAP[key] = value;
}
uint32_t INCREMENT(uint32_t key)
{
    
    MAP[key]++;
    return MAP[key];
}
uint32_t GET(uint32_t key)
{
    return MAP[key];
}
kv_t UGET(uint32_t ukey)
{
    kv_t kv;
    kv.key = ukey;
    kv.value = MAP[ukey];
    return kv;
}
void SETUP(size_t map_size, size_t sentinal)
{
    CLEAR();
    SIZE = map_size;
    SENTINEL = sentinal;
    MAP = mycalloc(SENTINEL+SIZE, sizeof(uint32_t));
}
void CLEAR()
{
    FREE(MAP);
    SIZE = 0;
    SENTINEL = 0;
}