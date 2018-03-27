#include "sparse_hash.h"

// typedef struct
// {
//     uint32_t max;
//     uint32_t ** values;
// } sparse_hash_t;
sparse_hash_t * sparse_hash_create(uint32_t max){
    sparse_hash_t * hash = mymalloc(sizeof(sparse_hash_t));
    size_t size = max >> HASHING_FACTOR;
    if(max % (1 << HASHING_FACTOR)) size++;
    hash->values = mycalloc(sizeof(uint32_t *), size);
    hash->max = max;
    return hash;
}
uint32_t sparse_hash_get(uint32_t key, sparse_hash_t * hash){
    if(hash->values[key >> HASHING_FACTOR] == 0) return 0;
    return hash->values[key >> HASHING_FACTOR][key % (1 << HASHING_FACTOR)];
}
uint32_t sparse_hash_set(uint32_t key, uint32_t value, sparse_hash_t * hash){
    if(hash->values[key >> HASHING_FACTOR] == 0) hash->values[key >> HASHING_FACTOR] = mycalloc(sizeof(uint32_t), (1 << HASHING_FACTOR));
    hash->values[key >> HASHING_FACTOR][key % (1 << HASHING_FACTOR)] = value;
    return hash->values[key >> HASHING_FACTOR][key % (1 << HASHING_FACTOR)];
}
uint32_t sparse_hash_increment(uint32_t key, uint32_t value, sparse_hash_t * hash){
    if(hash->values[key >> HASHING_FACTOR] == 0) hash->values[key >> HASHING_FACTOR] = mycalloc(sizeof(uint32_t), (1 << HASHING_FACTOR));
    hash->values[key >> HASHING_FACTOR][key % (1 << HASHING_FACTOR)] += value;
    return hash->values[key >> HASHING_FACTOR][key % (1 << HASHING_FACTOR)];
}
void sparse_hash_free(sparse_hash_t * hash){
    size_t size = hash->max / (1 << HASHING_FACTOR);
    if(hash->max % (1 << HASHING_FACTOR)) size++;
    for(uint i = 0; i < size; i++)
        if(hash->values[i] != 0)FREE(hash->values[i]);
    FREE(hash->values);
    FREE(hash);
}