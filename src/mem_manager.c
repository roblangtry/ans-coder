#include "mem_manager.h"
int64_t total_usage = 0;
int64_t max_usage = 0;
uint64_t ans_mem = 0;
uint64_t msb_mem = 0;
void * mymalloc(size_t size)
{
    void * ptr = malloc(size);
    if(ptr == NULL){
        fprintf(stderr, "Malloc failed on allocation of %zu (stack size at failure %ld)\n", size, total_usage);
        exit(-2);
    }
    total_usage += malloc_usable_size(ptr);
    if(total_usage > max_usage) max_usage = total_usage;
    return ptr;
}
void * mycalloc(size_t no, size_t size)
{
    void * ptr = calloc(no, size);
    if(ptr == NULL){
        fprintf(stderr, "Malloc failed on allocation of %zu (stack size at failure %ld)\n", size, total_usage);
        exit(-2);
    }
    total_usage += malloc_usable_size(ptr);
    if(total_usage > max_usage) max_usage = total_usage;
    return ptr;
}
void * myrealloc(void * ptr, size_t size)
{
    total_usage -= malloc_usable_size(ptr);
    void * nptr = realloc(ptr, size);
    if(nptr == NULL){
        fprintf(stderr, "Malloc failed on allocation of %zu (stack size at failure %ld)\n", size, total_usage);
        exit(-2);
    }
    total_usage += malloc_usable_size(nptr);
    if(total_usage > max_usage) max_usage = total_usage;
    return nptr;
}
void myfree(void * ptr)
{
    if(ptr != NULL){
        total_usage -= malloc_usable_size(ptr);
        free(ptr);
    }
}
void printmem()
{
    #if VERB_PROFILE
        fprintf(stderr, "MAX MEM USAGE:");
        fprintf(stderr, " %ld MiB", max_usage / 1048576);
        fprintf(stderr, " %ld KiB", (max_usage % 1048576) / 1024);
        fprintf(stderr, " %ld B\n", max_usage % 1024);
        fprintf(stderr, "ANS size: %ld\n", ans_mem);
        fprintf(stderr, "MSB size: %ld\n", msb_mem);
    #endif
    #if PROFILE
        fprintf(stderr, "%ld,%ld,%ld", max_usage, ans_mem, msb_mem);
    #endif
}
void ans_size(size_t size)
{
    ans_mem += size;
}
void msb_size(size_t size)
{
    msb_mem += size;
}