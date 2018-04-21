#include "order.h"

void build_translations_decoding(file_header_t * header,coding_signature_t signature, struct prelude_code_data * metadata)
{
    tuple_t * tuples = mymalloc(sizeof(tuple_t) * header->unique_symbols);
    uint32_t *sym = NULL, *freq = NULL, j=0, i=0;
    tuple_t *this = NULL, *top = NULL;
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
        this = tuples;
        top = tuples + header->unique_symbols;
        while(this<top)
        {
            j = get_symbol(++i, signature);
            header->symbol[j] = j;
            header->freq[j] += (*this++).freq;
        }
        header->freq[get_symbol(header->unique_symbols, signature)+1] = header->symbols;
        header->unique_symbols = get_symbol(header->unique_symbols, signature);
    }
    else
    {
        this = tuples;
        top = tuples + header->unique_symbols;
        sym = header->symbol;
        freq = header->freq;
        while(this<top)
        {
            (*sym++) = (*this).index+1;
            (*freq++) = (*this++).freq;
        }
        header->freq[header->unique_symbols] = header->symbols;

    }
    FREE(tuples);

}
void build_translations_encoding(file_header_t * header, uint32_t size, struct prelude_code_data * metadata,coding_signature_t signature)
{
    uint32_t no_unique = 0, symbol = 0, f = 0, max = 0, *sym = NULL, *top = NULL;
    uint32_t * F = NULL;
    kv_t kv;
    kv.key = 0;
    kv.value = 0;
    uint j = 0;
    SETUP(BLOCK_SIZE, header->Tmax);
    tuple_t *tuples = NULL, *this = NULL, *tip = NULL;
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
    tuple_t * tuples = mymalloc(sizeof(tuple_t) * no_unique);
    tuple_t *this = tuples, *top = tuples+no_unique;
    uint32_t *probe = get_map(), *base = NULL;
    base = probe;
    while(this<top){
        while(!(*probe)){
            probe++;
        }
        (*this).freq = *probe;
        (*this++).index = ((probe++) - base);
    }
    return tuples;
}
uint32_t * get_translation_matrix(tuple_t * tuples, uint32_t length, uint32_t max, file_header_t * header,coding_signature_t signature, struct prelude_code_data * metadata)
{
    uint32_t * T = mycalloc(max, sizeof(uint32_t));
    uint32_t raw = 0, symbol = 0, last = 0, nu = 0, *f, *f0, i=0;
    tuple_t *probe = NULL, *top = NULL;
    if(header->translation_mechanism == TRANSLATE_PARTIAL) ksort(tuples, length, header->translate_k);
    else qsort(tuples, length, sizeof(tuple_t), T_cmpfunc);
    probe = tuples;
    top = tuples+length;
    while(probe<top){
        raw = i + 1;
        if(header->translation_mechanism == TRANSLATE_PERMUTATION_PARTIAL)
        {
            if(i < header->translate_k){
                T[(*probe).index] = raw;
            }
            else{
                T[(*probe).index] = (*probe).index+header->translate_k;
            }
        }
        else T[(*probe).index] = raw;
        symbol = get_symbol(T[(*probe).index], signature);
        header->freq[symbol+1]+=(*probe).freq;
        if(symbol > header->max) header->max = symbol;
        if(symbol+1 != last)
        {
            nu++;
            last = symbol + 1;
        }
        probe++;
        i++;
    }
    if(perm_translating(header->translation_mechanism))
    {
        nu=0;
        f = header->freq;
        while(f<(header->freq+header->max+2)){
            if(*f++)
                nu++;
        }
        elias_encode(metadata, nu);
        header->nu = nu;
        f = header->freq;
        f0 = header->freq;
        symbol = 0;
        while(nu--)
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
            probe = tuples;
            top = tuples+length;
            while(probe<top)
                elias_encode(metadata, (*probe++).index);
        }
        else
        {
            nu = header->translate_k;
            if(length<nu) nu = length;
            elias_encode(metadata, nu);
            probe = tuples;
            top = tuples+nu;
            while(probe<top){
                elias_encode(metadata, (*probe++).index);
            }
        }
    }
    return T;
}
uint32_t * get_reverse_translation_matrix(tuple_t * tuples, uint32_t length, file_header_t * header, struct prelude_code_data * metadata)
{
    uint32_t *T = NULL, *TT = NULL, *To = NULL;
    if(full_translating(header->translation_mechanism)){
        T = mycalloc((length+1) , sizeof(uint32_t));
        To=T;
        TT=T+length;
        if(header->translation_mechanism == TRANSLATE_PARTIAL) ksort(tuples, length, header->translate_k);
        else qsort(tuples, length, sizeof(tuple_t), T_cmpfunc);
        while(T<TT){
            *(T++) = (*tuples++).index;
        }
    }
    else
    {
        length = elias_decode(metadata);
        T = mycalloc((length+1) , sizeof(uint32_t));
        To=T;
        TT=T+length;
        while(T<TT){
            *(T++) = elias_decode(metadata);
        }
    }
    return To;
}
void ksort(tuple_t * tuples, uint32_t length, uint32_t k)
{
    uint32_t val = 0;
    tuple_t *inner_probe = NULL, *outer_probe = NULL, *inner_limit = NULL, *outer_limit = NULL, *current_probe = NULL;
    tuple_t tup;
    tup.freq = 0;
    tup.index = 0;
    if(k>=length) return;
    outer_probe = tuples;
    outer_limit = tuples+k;
    inner_limit = tuples+length;
    while(outer_probe<outer_limit){
        val = (*outer_probe).freq;
        current_probe = outer_probe;
        inner_probe = outer_probe+1;
        while(inner_probe<inner_limit){
            if((*inner_probe).freq > val){
                val = (*inner_probe).freq;
                current_probe = inner_probe;
            }
            inner_probe++;
        }
        tup = (*outer_probe);
        (*outer_probe) = (*current_probe);
        (*current_probe) = tup;

    }
}
void SWAP(uint32_t * p1, uint32_t * p2)
{
    uint32_t t = *p1;
    *p1 = *p2;
    *p2 = t;
}