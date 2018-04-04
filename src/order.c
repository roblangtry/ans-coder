#include "order.h"

void build_translations_decoding(file_header_t * header,coding_signature_t signature)
{
    tuple_t * tuples = mymalloc(sizeof(tuple_t) * header->unique_symbols);
    for(uint32_t i = 0; i < header->unique_symbols; i++)
    {
        tuples[i].index = header->symbol[i];
        tuples[i].freq = header->freq[i];
        header->freq[i] = 0;
    }
    header->translation = get_reverse_translation_matrix(tuples, header->unique_symbols, header);
    for(uint32_t i = 0; i < header->unique_symbols; i++)
    {
        header->symbol[get_symbol(i+1, signature)] = get_symbol(i+1, signature);
        header->freq[get_symbol(i+1, signature)] += tuples[i].freq;
    }
    header->freq[get_symbol(header->unique_symbols, signature)+1] = header->symbols;
    header->unique_symbols = get_symbol(header->unique_symbols, signature);
    FREE(tuples);

}
void build_translations_encoding(file_header_t * header, uint32_t size, struct prelude_code_data * metadata,coding_signature_t signature)
{
    uint32_t no_unique = 0, symbol, f, max = 0;
    uint32_t * F = NULL;
    kv_t kv;
    uint j = 0;
    SETUP(BLOCK_SIZE, header->Tmax);
    tuple_t * tuples;
    for(uint i=0; i<size; i++)
    {
        symbol = header->data[i];
        f = INCREMENT(symbol);
        if(f == 1) no_unique++;
        if(symbol > max) max = symbol;
    }
    if(max > header->Tmax)
        header->Tmax = max;
    elias_encode(metadata, no_unique);
    tuples = mymalloc(sizeof(tuple_t) * no_unique);
    for(uint i=0; i<no_unique; i++)
    {
        kv = UGET(j++);
        while(kv.value <= 0){
            kv = UGET(j++);
        }
        elias_encode(metadata, kv.key);
        elias_encode(metadata, kv.value);
        tuples[i].freq = kv.value;
        tuples[i].index = kv.key;
    }
    header->nu = no_unique;
    FREE(F);
    header->translation = get_translation_matrix(tuples, no_unique, max + 1, header, signature);
    FREE(tuples);
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
        F = UGET(j).value;
        while(!F){
            j++;
            F = UGET(j).value;
        }
        tuples[i].freq = F;
        tuples[i].index = UGET(j++).key;
        // if(tuples[i].index == 131072) printf("T[%u] (%u,%u)\n", i , tuples[i].index, tuples[i].freq);
        i++;
    }
    return tuples;
}
uint32_t * get_translation_matrix(tuple_t * tuples, uint32_t length, uint32_t max, file_header_t * header,coding_signature_t signature)
{
    uint32_t * T = mycalloc(max, sizeof(uint32_t));
    uint32_t raw, symbol;
    if(header->translation_mechanism == TRANSLATE_PARTIAL) ksort(tuples, length, header->translate_k);
    else qsort(tuples, length, sizeof(tuple_t), T_cmpfunc);
    for(uint i=0; i<length; i++){
        raw = i + 1;
        T[tuples[i].index] = raw;
        symbol = get_symbol(raw, signature);
        header->freq[symbol+1]+=tuples[i].freq;
        if(symbol > header->max) header->max = symbol;
    }
    return T;
}
uint32_t * get_reverse_translation_matrix(tuple_t * tuples, uint32_t length, file_header_t * header)
{
    uint32_t * T = mycalloc((length+1) , sizeof(uint32_t));
    if(header->translation_mechanism == TRANSLATE_PARTIAL) ksort(tuples, length, header->translate_k);
    else qsort(tuples, length, sizeof(tuple_t), T_cmpfunc);
    for(uint i=0; i<length; i++)
        T[i] = tuples[i].index;
    return T;
}
void ksort(tuple_t * tuples, uint32_t length, uint32_t k)
{
    size_t i,j,current;
    uint32_t val;
    tuple_t tup;
    if(k>=length) return;
    for(i = 0; i < k; i++){
        val = tuples[i].freq;
        current = i;
        for(j = i + 1; j < length; j++){
            if(tuples[j].freq > val){
                val = tuples[j].freq;
                current = j;
            }
        }
        tup = tuples[i];
        tuples[i] = tuples[current];
        tuples[current] = tup;

    }
}

void kcheck(size_t i, size_t * top, uint32_t k, tuple_t * tuples)
{
    size_t temp;
    for(int j = k-1; j >= 0; j--)
    {
        if(tuples[i].freq > tuples[top[j]].freq){
            temp = i;
            i = top[j];
            top[j] = temp;
        }
        else
            break;
    }
}