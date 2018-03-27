#include "page.h"


int_page_t * get_int_page()
{
    int_page_t * page = mymalloc(sizeof(int_page_t));
    page->data = mymalloc(sizeof(uint32_t) * PAGE_SIZE);
    page->size = PAGE_SIZE;
    page->current_size = 0;
    return page;
}
void add_to_int_page(uint32_t value, int_page_t * page)
{
    if(page->current_size >= page->size){
        page->data = (uint32_t *)myrealloc(page->data, (page->size + PAGE_SIZE)*sizeof(uint32_t));
        page->size = page->size + PAGE_SIZE;
    }
    page->data[page->current_size++] = value;

}
void output_int_page(struct writer * my_writer, int_page_t * page, uint32_t bits)
{
    struct bit_writer * bwriter = initialise_bit_writer(my_writer);
    for(uint i = 0; i < page->current_size; i++)
        write_bits(page->data[i], bits, bwriter);
    flush_bit_writer(bwriter);
    free_bit_writer(bwriter);
}
void free_int_page(int_page_t * page)
{
    myfree(page->data);
    myfree(page);
}


char_page_t * get_char_page()
{
    char_page_t * page = mymalloc(sizeof(char_page_t));
    page->data = mymalloc(sizeof(unsigned char) * PAGE_SIZE);
    page->size = PAGE_SIZE;
    page->current_size = 0;
    return page;
}
void add_to_char_page(unsigned char value, char_page_t * page)
{
    if(page->current_size >= page->size){
        page->data = (unsigned char *)myrealloc(page->data, (page->size + PAGE_SIZE)*sizeof(unsigned char));
        page->size = page->size + PAGE_SIZE;
    }
    page->data[page->current_size++] = value;

}

void output_char_page(struct writer * my_writer, char_page_t * page)
{
    write_bytes(page->data, page->current_size, my_writer);
}


void free_char_page(char_page_t * page)
{
    myfree(page->data);
    myfree(page);
}


bit_page_t * get_bit_page()
{
    bit_page_t * page = mymalloc(sizeof(bit_page_t));
    page->data = mymalloc(sizeof(bits_t) * PAGE_SIZE);
    page->size = PAGE_SIZE;
    page->current_size = 0;
    return page;
}
void add_to_bit_page(uint32_t value, unsigned char length, bit_page_t * page)
{
    if(page->current_size >= page->size){
        page->data = (bits_t *)myrealloc(page->data, (page->size + PAGE_SIZE)*sizeof(bits_t));
        page->size = page->size + PAGE_SIZE;
    }
    page->data[page->current_size].data = value;
    page->data[page->current_size].length = length;
    page->current_size++;
}
void output_bit_page(struct writer * my_writer, bit_page_t * page)
{
    struct bit_writer * bwriter = initialise_bit_writer(my_writer);
    for(uint i = 0; i < page->current_size; i++){
        write_bits(page->data[i].data, page->data[i].length, bwriter);
    }
    flush_bit_writer(bwriter);
    free_bit_writer(bwriter);
}
void free_bit_page(bit_page_t * page)
{
    myfree(page->data);
    myfree(page);
}