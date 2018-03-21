#include "order.h"

void build_translations_encoding(file_header_t * header, uint32_t size, struct prelude_code_data * metadata)
{
    uint32_t no_unique = 0, symbol, f, max = 0;
    uint32_t * F = mycalloc(SYMBOL_MAP_SIZE , sizeof(uint32_t));
    tuple_t * tuples;
    for(uint i=0; i<size; i++)
    {
        symbol = header->data[i];
        if(!F[symbol]) no_unique++;
        F[symbol]++;
        if(symbol > max) max = symbol;
    }
    elias_encode(metadata, no_unique);
    uint j = 0;
    for(uint i=0; i<no_unique; i++)
    {
        f = F[j];
        while(f <= 0){
            j++;
            f = F[j];
        }
        elias_encode(metadata, j - symbol);
        symbol = j++;
        elias_encode(metadata, f);
    }
    tuples = get_tuples(F, no_unique);
    myfree(F);
    header->translation = get_translation_matrix(tuples, no_unique, max + 1);
    myfree(tuples);
}

int T_cmpfunc (const void * a, const void * b) {
   return ( (*(tuple_t*)b).freq - (*(tuple_t*)a).freq );
}

tuple_t * get_tuples(uint32_t * freq, uint32_t no_unique)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t F;
    tuple_t * tuples = mymalloc(sizeof(tuple_t) * no_unique);
    while(i < no_unique){
        F = freq[j];
        while(!F){
            j++;
            F = freq[j];
        }
        tuples[i].freq = F;
        tuples[i].index = j++;
        i++;
    }
    return tuples;
}
uint32_t * get_translation_matrix(tuple_t * tuples, uint32_t length, uint32_t max)
{
    uint32_t * T = mycalloc(max, sizeof(uint32_t));
    qsort(tuples, length, sizeof(tuple_t), T_cmpfunc);
    for(uint i=0; i<length; i++)
        T[tuples[i].index] = i + 1;
    return T;
}
uint32_t * get_reverse_translation_matrix(tuple_t * tuples, uint32_t length, uint32_t max)
{
    uint32_t * T = mycalloc(length, sizeof(uint32_t));
    qsort(tuples, length, sizeof(tuple_t), T_cmpfunc);
    for(uint i=0; i<length; i++)
        T[i] = tuples[i].index;
    return T;
}