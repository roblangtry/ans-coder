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
void output_int_page(struct writer * my_writer, int_page_t * page)
{
    write_bytes((unsigned char *)page->data, page->current_size << 2, my_writer);
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