#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include "constants.h"
#ifndef MEM_MNGR_CODE
#define MEM_MNGR_CODE
#define FREE(ptr) do{ \
    myfree((ptr));      \
    (ptr) = NULL;     \
  }while(0)
void * mycalloc(size_t no, size_t size);
void * mymalloc(size_t size);
void * myrealloc(void * ptr, size_t size);
void myfree(void * ptr);
void printmem();
void ans_size(size_t size);
void msb_size(size_t size);
#endif