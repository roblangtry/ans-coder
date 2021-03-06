#include "writer.h"
//writer functions
struct writer * initialise_writer(FILE * output_file)
{
    
    struct writer * my_writer = (struct writer *)mymalloc(sizeof(struct writer));
    my_writer->index = 0;
    my_writer->buffer = mymalloc(sizeof(unsigned char) * WRITE_BUFFER);
    my_writer->output_file = output_file;
    return my_writer;
}
uint64_t write_byte(unsigned char byte, struct writer * my_writer)
{
    my_writer->buffer[my_writer->index++] = byte;
    if(my_writer->index >= WRITE_BUFFER){
        my_writer->index = 0;
        return fwrite(my_writer->buffer, sizeof(unsigned char), WRITE_BUFFER, my_writer->output_file);
    }
    return 0;
}
uint64_t write_uint32_t(uint32_t number, struct writer * my_writer)
{
    return write_bytes((unsigned char *)&number, 4, my_writer);
}
uint64_t write_uint64_t(uint64_t number, struct writer * my_writer)
{
    return write_bytes((unsigned char *)&number, 8, my_writer);
}
uint64_t write_bytes(unsigned char * byte_array, size_t no_bytes, struct writer * my_writer)
{
    while(my_writer->index+no_bytes >= WRITE_BUFFER){
        memcpy(my_writer->buffer+my_writer->index, byte_array, WRITE_BUFFER-my_writer->index);
        byte_array = byte_array+WRITE_BUFFER-my_writer->index;
        no_bytes -= WRITE_BUFFER-my_writer->index;
        my_writer->index = 0;
        fwrite(my_writer->buffer, sizeof(unsigned char), WRITE_BUFFER, my_writer->output_file);
    }
    memcpy(my_writer->buffer+my_writer->index, byte_array, no_bytes);
    my_writer->index += no_bytes;
    return 0;
}
void reverse_write_bytes(unsigned char * byte_array, size_t no_bytes, struct writer * my_writer)
{
    unsigned char *probe = byte_array+no_bytes-1;
    while(probe >= byte_array){
        write_byte((*probe--), my_writer);
    }
}
uint64_t flush_writer(struct writer * my_writer)
{
    uint64_t write_amount = 0;
    if(my_writer->index == 0) return 0;
    else {
        write_amount = fwrite(my_writer->buffer, sizeof(unsigned char), my_writer->index, my_writer->output_file);
        my_writer->index = 0;
        return write_amount;
    }
}
//bit writer functions
struct bit_writer * initialise_bit_writer(struct writer * my_writer)
{
    struct bit_writer * my_bit_writer = (struct bit_writer *)mymalloc(sizeof(struct bit_writer));
    my_bit_writer->my_writer = my_writer;
    my_bit_writer->buffer = 0;
    my_bit_writer->length = 0;
    return my_bit_writer;
}
void free_bit_writer(struct bit_writer * my_bit_writer)
{
    FREE(my_bit_writer);
}
uint64_t write_bits(uint64_t value, uint64_t length, struct bit_writer * my_bit_writer)
{
    unsigned char byte = 0;
    uint64_t write_amount = 0;
    my_bit_writer->buffer = my_bit_writer->buffer << length;
    my_bit_writer->buffer += value;
    my_bit_writer->length += length;
    while(my_bit_writer->length >= 8){
        my_bit_writer->length -= 8;
        byte = my_bit_writer->buffer >> my_bit_writer->length;
        my_bit_writer->buffer = my_bit_writer->buffer % (1 << my_bit_writer->length);//- (byte << my_bit_writer->length);
        write_amount += write_byte(byte, my_bit_writer->my_writer);
    }
    return write_amount;


}
uint64_t flush_bit_writer(struct bit_writer * my_bit_writer)
{
    uint64_t write_amount = 0;
    unsigned char byte = 0;
    while(my_bit_writer->length >= 8){
        my_bit_writer->length -= 8;
        byte = my_bit_writer->buffer >> my_bit_writer->length;
        my_bit_writer->buffer = my_bit_writer->buffer - (byte << my_bit_writer->length);
        write_amount += write_byte(byte, my_bit_writer->my_writer);
    }
    if(my_bit_writer->length > 0){
        byte = my_bit_writer->buffer << (8 - my_bit_writer->length);
        my_bit_writer->buffer = 0;
        my_bit_writer->length = 0;
        write_amount += write_byte(byte, my_bit_writer->my_writer);
    }
    return write_amount;
}