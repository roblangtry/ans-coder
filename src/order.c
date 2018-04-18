#include "order.h"

void build_translations_decoding(file_header_t * header,coding_signature_t signature, struct prelude_code_data * metadata)
{
    tuple_t * tuples = mymalloc(sizeof(tuple_t) * header->unique_symbols);
    uint32_t *sym, *freq, j=0;
    tuple_t *this, *top;
    this = tuples;
    top = tuples + header->unique_symbols;
    sym = header->symbol;
    freq = header->freq;
    while(this < top)
    {
        (*this).index = (*sym++);
        (*this++).freq = (*freq);
        (*freq++) = 0;
    }
    header->translation = get_reverse_translation_matrix(tuples, header->unique_symbols, header, metadata);
    if(full_translating(header->translation_mechanism))
    {
        for(uint32_t i = 0; i < header->unique_symbols; i++)
        {
            header->symbol[get_symbol(i+1, signature)] = get_symbol(i+1, signature);
            header->freq[get_symbol(i+1, signature)] += tuples[i].freq;
        }
        header->freq[get_symbol(header->unique_symbols, signature)+1] = header->symbols;
        header->unique_symbols = get_symbol(header->unique_symbols, signature);
    }
    else
    {
        for(uint32_t i = 0; i < header->unique_symbols; i++)
        {
            header->symbol[i] = tuples[i].index+1;
            header->freq[i] = tuples[i].freq;
        }
        header->freq[header->unique_symbols] = header->symbols;

    }
    FREE(tuples);

}
void build_translations_encoding(file_header_t * header, uint32_t size, struct prelude_code_data * metadata,coding_signature_t signature)
{
    uint32_t no_unique = 0, symbol, f, max = 0, *sym, *top;
    uint32_t * F = NULL;
    kv_t kv;
    uint j = 0;
    SETUP(BLOCK_SIZE, header->Tmax);
    tuple_t * tuples, *this, *tip;
    sym = header->data;
    top = header->data + size;
    while(sym < top)
    {
        symbol = (*sym++);
        f = INCREMENT(symbol);
        if(f == 1) no_unique++;
        if(symbol > max) max = symbol;
    }
    if(max > header->Tmax)
        header->Tmax = max;
    if(full_translating(header->translation_mechanism)) elias_encode(metadata, no_unique);
    tuples = mymalloc(sizeof(tuple_t) * no_unique);
    symbol = 0;
    this = tuples;
    tip = tuples + no_unique;
    while(this < tip)
    {
        kv = UGET(j++);
        while(kv.value <= 0){
            kv = UGET(j++);
        }
        if(full_translating(header->translation_mechanism)){
            elias_encode(metadata, kv.key - symbol);
            elias_encode(metadata, kv.value);
        }
        symbol = kv.key;
        (*this).freq = kv.value;
        (*this++).index = kv.key;
    }
    header->nu = no_unique;
    FREE(F);
    header->translation = get_translation_matrix(tuples, no_unique, max + 1, header, signature, metadata);
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
        i++;
    }
    return tuples;
}
uint32_t * get_translation_matrix(tuple_t * tuples, uint32_t length, uint32_t max, file_header_t * header,coding_signature_t signature, struct prelude_code_data * metadata)
{
    uint32_t * T = mycalloc(max, sizeof(uint32_t));
    uint32_t raw, symbol, last = 0, nu = 0, *f, *f0;
    if(header->translation_mechanism == TRANSLATE_PARTIAL) ksort(tuples, length, header->translate_k);
    else qsort(tuples, length, sizeof(tuple_t), T_cmpfunc);
    for(uint i=0; i<length; i++){
        raw = i + 1;
        if(header->translation_mechanism == TRANSLATE_PERMUTATION_PARTIAL)
        {
            if(i < header->translate_k){
                // if(tuples[i].index == 3380) printf("T [%u] = %u\n", tuples[i].index, raw);
                T[tuples[i].index] = raw;
            }
            else{
                // if(tuples[i].index == 3380) printf("T2[%u] = %u\n", tuples[i].index, tuples[i].index+header->translate_k);
                T[tuples[i].index] = tuples[i].index+header->translate_k;
            }
        }
        else T[tuples[i].index] = raw;
        symbol = get_symbol(T[tuples[i].index], signature);
        header->freq[symbol+1]+=tuples[i].freq;
        if(symbol > header->max) header->max = symbol;
        if(symbol+1 != last)
        {
            nu++;
            last = symbol + 1;
        }
    }
    if(perm_translating(header->translation_mechanism))
    {
        elias_encode(metadata, nu);
        header->nu = nu;
        f = header->freq;
        f0 = header->freq;
        symbol = 0;
        for(uint i=0;i<nu;i++)
        {
            while(!(*f))
                f++;
            elias_encode(metadata, (f-f0-2)-symbol);
            symbol = (f-f0-2);
            elias_encode(metadata, *f);
            f++;
        }
        if(header->translation_mechanism == TRANSLATE_PERMUTATION_TRUE)
        {
            elias_encode(metadata, length);
            for(uint i=0; i<length;i++)
                elias_encode(metadata, tuples[i].index);
        }
        else
        {
            nu = header->translate_k;
            if(length<nu) nu = length;
            elias_encode(metadata, nu);
            for(uint i=0; i<nu;i++){
                elias_encode(metadata, tuples[i].index);
            }
        }
    }
    return T;
}
uint32_t * get_reverse_translation_matrix(tuple_t * tuples, uint32_t length, file_header_t * header, struct prelude_code_data * metadata)
{
    uint32_t * T;
    if(full_translating(header->translation_mechanism)){
        T = mycalloc((length+1) , sizeof(uint32_t));
        if(header->translation_mechanism == TRANSLATE_PARTIAL) ksort(tuples, length, header->translate_k);
        else qsort(tuples, length, sizeof(tuple_t), T_cmpfunc);
        for(uint i=0; i<length; i++){
            T[i] = tuples[i].index;
        }
    }
    else
    {
        length = elias_decode(metadata);
        T = mycalloc((length+1) , sizeof(uint32_t));
        for(uint i=0; i<length; i++){
            T[i] = elias_decode(metadata);
        }
    }
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
void SWAP(uint32_t * p1, uint32_t * p2)
{
    uint32_t t = *p1;
    *p1 = *p2;
    *p2 = t;
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