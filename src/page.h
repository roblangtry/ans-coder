#include <stdint.h>
#include <stdio.h>
#include "constants.h"
#include "mem_manager.h"
#include "writer.h"
#ifndef PAGE_CODE
#define PAGE_CODE

typedef struct {
    uint32_t data;
    unsigned char length;
} bits_t;
typedef struct
{
    bits_t * data;
    size_t size;
    size_t current_size;
} bit_page_t;
bit_page_t * get_bit_page();
void add_to_bit_page(uint32_t value, unsigned char length, bit_page_t * page);
void output_bit_page(struct writer * my_writer, bit_page_t * page);
void free_bit_page(bit_page_t * page);
typedef struct
{
    uint32_t * data;
    size_t size;
    size_t current_size;
    uint64_t state;
    size_t length;
    uint64_t no_writes;
} bint_page_t;
bint_page_t * get_bint_page();
void add_to_bint_page(uint32_t value, size_t length, bint_page_t * page);
void output_bint_page(struct writer * my_writer, bint_page_t * page, uint32_t B);
void free_bint_page(bint_page_t * page);
#endif