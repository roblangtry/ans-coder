#include "priority.h"
//struct heap_node {
//    float v;
//    uint symbol;
//}
//struct heap {
//    struct heap_node * data;
//    uint elements;
//}
struct heap initialise_heap(uint size){
    struct heap heap;
    heap.data = mymalloc(sizeof(struct heap_node) * (size + 1));
    heap.elements = 0;
    return heap;
}
void H_put(struct heap * heap, struct heap_node node){
    uint current, parent;
    struct heap_node intermediate;
    heap->elements += 1;
    heap->data[heap->elements] = node;
    current = heap->elements;
    parent = current / 2;
    while(current > 0){
        if(heap->data[current].v < heap->data[parent].v){
            intermediate = heap->data[parent];
            heap->data[parent] = heap->data[current];
            heap->data[current] = intermediate;
        } else{
            current = 0;
        }
    }
}
struct heap_node H_pop(struct heap * heap){
    uint current, child1, child2, stop;
    struct heap_node intermediate, node;
    node = heap->data[1];
    heap->elements -= 1;
    current = 1;
    child1 = 2;
    child2 = 3;
    stop = 0;
    while(stop == 0){
        if(current >= heap->elements){
            stop = 1;
        }
        else if(current + 2 <= heap->elements){
            if(heap->data[current].v > heap->data[child1].v){
                if(heap->data[child1].v > heap->data[child2].v){
                    intermediate = heap->data[child2];
                    heap->data[child2] = heap->data[current];
                    heap->data[current] = intermediate;
                    current = child2;
                    child1 = current * 2;
                    child2 = current * 2 + 1;
                }else{
                    intermediate = heap->data[child1];
                    heap->data[child1] = heap->data[current];
                    heap->data[current] = intermediate;
                    current = child1;
                    child1 = current * 2;
                    child2 = current * 2 + 1;
                }
            } else if(heap->data[current].v > heap->data[child2].v){
                intermediate = heap->data[child2];
                heap->data[child2] = heap->data[current];
                heap->data[current] = intermediate;
                current = child2;
                child1 = current * 2;
                child2 = current * 2 + 1;
            }else{
                stop = 1;
            }
        }
        else
        {
            if(heap->data[current].v > heap->data[child1].v){
                intermediate = heap->data[child1];
                heap->data[child1] = heap->data[current];
                heap->data[current] = intermediate;
                current = child1;
                child1 = current * 2;
                child2 = current * 2 + 1;
            }
            else
            {
                stop = 1;
            }
        }
    }
    return node;
}