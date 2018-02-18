#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "mem_manager.h"
#ifndef Q_CODE
#define Q_CODE
struct heap_node {
    float v;
    uint64_t s;
};
struct heap {
    struct heap_node * data;
    uint elements;
};
struct heap initialise_heap(uint size);
void H_put(struct heap * heap, struct heap_node node);
struct heap_node H_pop(struct heap * heap);
#endif