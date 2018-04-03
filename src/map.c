#include "map.h"
kv_t * MAP = NULL;
kv_t * DENSE = NULL;
size_t SIZE = 0;
size_t SENTINEL = 0;
size_t PAGES = 0;
void ADD(uint32_t key,uint32_t value)
{
    if(key> SENTINEL){
        DENSE[key - SENTINEL].key = key;
        DENSE[key - SENTINEL].value = value;
    }
    else
    {
        uint o = 0;
        uint k = (key*12289);
        while(MAP[(k+o) % SIZE].key != 0 && MAP[(k+o) % SIZE].key != key){
            o++;
        }
        MAP[(k+o) % SIZE].key = key;
        MAP[(k+o) % SIZE].value = value;
    }
}
uint32_t INCREMENT(uint32_t key)
{
    if(key> SENTINEL){
        DENSE[key - SENTINEL].key = key;
        DENSE[key - SENTINEL].value++;
        return DENSE[key - SENTINEL].value;
    }
    else
    {
        uint o = 0;
        uint k = (key*12289);
        while(MAP[(k+o) % SIZE].key != 0 && MAP[(k+o) % SIZE].key != key){
            o++;
        }
        MAP[(k+o) % SIZE].key = key;
        MAP[(k+o) % SIZE].value++;
        return MAP[(k+o) % SIZE].value;
    }
}
uint32_t GET(uint32_t key)
{
    if(key> SENTINEL){
        return DENSE[key - SENTINEL].value;
    }
    else
    {
        uint o = 0;
        uint k = (key*12289);
        while(MAP[(k+o) % SIZE].key != 0 && MAP[(k+o) % SIZE].key != key){
            o++;
        }
        return MAP[(k+o) % SIZE].value;
    }
}
kv_t UGET(uint32_t ukey)
{
    if(ukey >= SIZE) return DENSE[ukey - SIZE];
    return MAP[ukey % SIZE];
}
void SETUP(size_t map_size, size_t sentinal)
{
    CLEAR();
    SIZE = map_size;
    SENTINEL = sentinal;
    MAP = mycalloc(map_size, sizeof(kv_t));
    DENSE = mycalloc(map_size, sizeof(kv_t));
}
void CLEAR()
{
    FREE(MAP);
    FREE(DENSE);
    SIZE = 0;
    SENTINEL = 0;
}