#include "page.h"




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
    bits_t *probe = page->data, *top = page->data+page->current_size;
    struct bit_writer * bwriter = initialise_bit_writer(my_writer);
    while(probe<top){
        write_bits((*probe).data, (*probe).length, bwriter);
        probe++;
    }
    flush_bit_writer(bwriter);
    free_bit_writer(bwriter);
}
void free_bit_page(bit_page_t * page)
{
    FREE(page->data);
    FREE(page);
}


bint_page_t * get_bint_page()
{
    bint_page_t * page = mymalloc(sizeof(bint_page_t));
    page->data = mymalloc(sizeof(uint32_t) * PAGE_SIZE);
    page->size = PAGE_SIZE;
    page->current_size = 0;
    page->state = 0;
    page->length = 0;
    page->no_writes = 0;
    return page;
}
void add_to_bint_page(uint32_t value, size_t length, bint_page_t * page)
{
    uint64_t V = 0;
    // printf("+ %u[%u]\n", value, length);
    // printf("%u", page->state);
    page->state = (page->state << length) + value;
    // printf(" -> %u", page->state);
    page->length += length;
    // printf("[%u]\n", page->length);
    // sleep(1);
    if(page->length>=31)
    {
        if(page->length==31){
            V = page->state;
            page->state = 0;
            page->length = 0;
        }
        else{
            V = page->state >> (page->length - 31);
            page->state = page->state % (1 << (page->length - 31));
            page->length -= 31;
        }
        if(page->current_size >= page->size){
            page->data = (uint32_t *)myrealloc(page->data, (page->size + PAGE_SIZE)*sizeof(uint32_t));
            page->size = page->size + PAGE_SIZE;
        }
        page->data[page->current_size++] = (uint32_t)V;
    }
    page->no_writes++;
}
void output_bint_page(struct writer * my_writer, bint_page_t * page, uint32_t bits)
{
    uint32_t *head=page->data, *max=page->data+page->current_size;
    struct bit_writer * bwriter = initialise_bit_writer(my_writer);
    while(head < max)
        write_bits(*head++, 31, bwriter);
    write_bits(page->state, page->length, bwriter);
    flush_bit_writer(bwriter);
    free_bit_writer(bwriter);
}
void free_bint_page(bint_page_t * page)
{
    FREE(page->data);
    FREE(page);
}
