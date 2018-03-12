#include "reader.h"
//reader functions
struct reader * initialise_reader(FILE * input_file){
    struct reader * my_reader = mymalloc(sizeof(struct reader));
    my_reader->index = 0;
    my_reader->size = 0;
    my_reader->input_file = input_file;
    return my_reader;
}
size_t read_byte(unsigned char * target, struct reader * my_reader){
    if(my_reader->index < my_reader->size){
        (*target) = my_reader->buffer[my_reader->index];
        my_reader->index++;
        return 1;
    } else {
        my_reader->index = 0;
        my_reader->size = fread(my_reader->buffer, sizeof(unsigned char), READ_BUFFER, my_reader->input_file);
        if(my_reader->size == 0){
            return 0; // EOF
        } else {
            (*target) = my_reader->buffer[my_reader->index];
            my_reader->index++;
            return 1;
        }
    }
}
size_t read_uint32_t(uint32_t * target, struct reader * my_reader){
    read_bytes(target, 4, my_reader);
}
size_t read_uint64_t(uint64_t * target, struct reader * my_reader){
    read_bytes(target, 8, my_reader);
}
size_t read_bytes(unsigned char * target, size_t no_bytes, struct reader * my_reader){
    size_t index = 0;
    while(index < no_bytes){
        if(read_byte(target+index, my_reader) == 0) break;
        else index++;
    }
    return index;
}
//bit reader functions
struct bit_reader * initialise_bit_reader(struct reader * my_reader)
{
    struct bit_reader * my_bit_reader = mymalloc(sizeof(struct bit_reader));
    my_bit_reader->my_reader = my_reader;
    my_bit_reader->buffer = 0;
    my_bit_reader->length = 0;
    return my_bit_reader;
}
void free_bit_reader(struct bit_reader * my_bit_reader)
{
    myfree(my_bit_reader);
}
uint64_t read_bits(uint64_t length, struct bit_reader * my_bit_reader){
    unsigned char byte;
    uint64_t value;
    uint64_t diff;
    while(length > my_bit_reader->length){
        if (read_byte(&byte, my_bit_reader->my_reader) == 0) byte = 0;
        my_bit_reader->buffer = (my_bit_reader->buffer << 8) + byte;
        my_bit_reader->length = my_bit_reader->length + 8;
    }
    diff = my_bit_reader->length - length;
    value = my_bit_reader->buffer >> diff;
    my_bit_reader->buffer = my_bit_reader->buffer - (value << diff);
    my_bit_reader->length = my_bit_reader->length - length;
    return value;
}
