#include <stdint.h>
#include <stdio.h>
#include "constants.h"
#include "mem_manager.h"
#include "writer.h"
#ifndef PAGE_CODE
#define PAGE_CODE
typedef struct
{
    uint32_t * data;
    size_t size;
    size_t current_size;
} int_page_t;
int_page_t * get_int_page();
void add_to_int_page(uint32_t value, int_page_t * page);
void output_int_page(struct writer * my_writer, int_page_t * page);
void free_int_page(int_page_t * page);

typedef struct
{
    unsigned char * data;
    size_t size;
    size_t current_size;
} char_page_t;
char_page_t * get_char_page();
void add_to_char_page(unsigned char value, char_page_t * page);
void output_char_page(struct writer * my_writer, char_page_t * page);
void free_char_page(char_page_t * page);
#endif