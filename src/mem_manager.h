#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#ifndef MEM_MNGR_CODE
#define MEM_MNGR_CODE
void * mymalloc(size_t size);
void myfree(void * ptr);
void printmem();
#endif