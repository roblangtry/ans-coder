#include "mem_manager.h"
int64_t total_usage = 0;
int64_t max_usage = 0;
void * mymalloc(size_t size)
{
    void * ptr = malloc(size);
    total_usage += malloc_usable_size(ptr);
    if(total_usage > max_usage) max_usage = total_usage;
    return ptr;
}
void myfree(void * ptr)
{
    total_usage -= malloc_usable_size(ptr);
    free(ptr);
}
void printmem()
{
    fprintf(stderr, "MAX MEM USAGE: %ld\n", max_usage);
}