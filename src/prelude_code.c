#include "prelude_code.h"


struct prelude_code_data * prepare_metadata(struct reader * reader_ptr, struct writer * writer_ptr, uint64_t initial_state){
    struct prelude_code_data * metadata = (struct prelude_code_data *)malloc(sizeof(struct prelude_code_data));
    metadata->state = initial_state;
    metadata->lo = 0;
    metadata->hi = 0;
    metadata->index = 0;
    if(writer_ptr != NULL){
        metadata->writer_ptr = writer_ptr;
        metadata->bit_writer_ptr = initialise_bit_writer(metadata->writer_ptr);
    }
    else {
        metadata->writer_ptr = NULL;
        metadata->bit_writer_ptr = NULL;
    }
    if(reader_ptr != NULL){
        metadata->reader_ptr = reader_ptr;
        metadata->bit_reader_ptr = initialise_bit_reader(metadata->reader_ptr);
    }
    else {
        metadata->reader_ptr = NULL;
        metadata->bit_reader_ptr = NULL;
    }
    return metadata;
}
// ---------------
// Variable byte
// ---------------
void vbyte_encode(struct prelude_code_data * metadata, uint64_t value){
    uint64_t cur_value = value;
    unsigned char byte;
    while(cur_value >= 128){
        byte = cur_value % 128;
        write_byte(byte, metadata->writer_ptr);
        cur_value = cur_value >> 7;
    }
    byte = cur_value + 128;
    write_byte(byte, metadata->writer_ptr);
}
void vbyte_flush(struct prelude_code_data * metadata){
    //flush_writer(metadata->writer_ptr);
}
uint64_t vbyte_decode(struct prelude_code_data * metadata){
    uint64_t value = 0;
    uint round = 0;
    unsigned char byte;
    read_byte(&byte, metadata->reader_ptr);
    while(byte < 128){
        value  = value + (byte << (7 * round));
        read_byte(&byte, metadata->reader_ptr);
        round++;
    }
    value  = value + ((byte - 128) << (7 * round));
    return value;
}
// ---------------
// Elias
// ---------------
void elias_encode(struct prelude_code_data * metadata, uint64_t value){
    uint64_t zlength = ((int)log2(value));
    uint64_t length = zlength + 1;
    write_bits(0, zlength, metadata->bit_writer_ptr);
    write_bits(value, length, metadata->bit_writer_ptr);
}
void elias_flush(struct prelude_code_data * metadata){
    flush_bit_writer(metadata->bit_writer_ptr);
}
uint64_t elias_decode(struct prelude_code_data * metadata){
    uint no_zeroes = 0;
    uint64_t value = 0;
    while(value == 0){
        value += read_bits(1, metadata->bit_reader_ptr);
        if(value == 0) no_zeroes++;
    }
    value = value << no_zeroes;
    if(no_zeroes > 0) value += read_bits(no_zeroes, metadata->bit_reader_ptr);
    return value;
}
// ---------------
// Golomb
// ---------------
void golomb_encode(struct prelude_code_data * metadata, uint64_t value)
{

}
void golomb_flush(struct prelude_code_data * metadata)
{

}
uint64_t golomb_decode(struct prelude_code_data * metadata)
{

}

// ---------------
// Two-pass
// ---------------
void two_pass_store(struct prelude_code_data * metadata, uint64_t value)
{
    if(metadata->index == 0){
        metadata->lo = value;
        metadata->hi = value;
    }
    if(metadata->lo > value) metadata->lo = value;
    if(metadata->hi < value) metadata->hi = value;
    metadata->buffer[metadata->index] = value;
    metadata->index += 1;
}
// ---------------
// Interpolative
// ---------------
void interp_encode(struct prelude_code_data * metadata, uint64_t value)
{
    two_pass_store(metadata, value);
}
void interp_flush(struct prelude_code_data * metadata)
{

}
uint64_t interp_decode(struct prelude_code_data * metadata)
{

}
// ---------------
// ANS coder
// ---------------
void ans_encode(struct prelude_code_data * metadata, uint64_t value)
{
    two_pass_store(metadata, value);
}
void ans_flush(struct prelude_code_data * metadata)
{
    size_t output_mod = 1;
    size_t output_index = 0;
    size_t meta_index = 0;
    size_t size = PRELUDE_BUFFER_SIZE * output_mod * sizeof(unsigned char);
    unsigned char * output = (unsigned char *)malloc(size);
    uint64_t b = 256;
    uint64_t m = metadata->hi - metadata->lo + 1;
    uint64_t x = m;
    uint64_t ls;
    uint64_t bs;
    uint64_t s;
    unsigned char byte;
    while(meta_index != metadata->index)
    {
        if((size - 10) <= output_index)
        {
            output_mod += 1;
            size = PRELUDE_BUFFER_SIZE * output_mod * sizeof(unsigned char);
            realloc(output, size);
        }
        s = metadata->buffer[metadata->index - meta_index - 1];
        ls = 1;
        bs = s - metadata->lo;
        while(x > (ls * (b-1)))
        {
            byte = x % b;
            output[output_index] = byte;
            output_index += 1;
            x = x / b;
        }
        x = (m * (x / ls)) + bs + (x % ls);
        meta_index += 1;
    }
    vbyte_encode(metadata, metadata->lo);
    vbyte_encode(metadata, metadata->hi);
    vbyte_encode(metadata, x);
    vbyte_encode(metadata, metadata->index);
    vbyte_flush(metadata);
    meta_index = 0;
    while(meta_index < output_index){
        byte = output[output_index - meta_index - 1];
        write_byte(byte, metadata->writer_ptr);
        meta_index += 1;
    }
}
uint64_t ans_decode(struct prelude_code_data * metadata)
{
    uint64_t value;
    if(metadata->hi == 0 && metadata->lo == 0) get_ans_data(metadata);
    value = metadata->buffer[metadata->index];
    metadata->index += 1;
    return value;
}
void get_ans_data(struct prelude_code_data * metadata)
{
    uint64_t lo = vbyte_decode(metadata);
    uint64_t hi = vbyte_decode(metadata);
    uint64_t x = vbyte_decode(metadata);
    uint64_t m = hi - lo + 1;
    uint64_t s;
    uint64_t ls;
    uint64_t bs;
    uint64_t b = 256;
    size_t num = vbyte_decode(metadata);
    size_t i = 0;
    unsigned char byte;
    metadata->state = num;
    metadata->index = 0;
    metadata->lo = lo;
    metadata->hi = hi;
    while(i < num)
    {
        s = x % m + lo;
        ls = 1;
        bs = s - lo;
        metadata->buffer[i] = s;
        i += 1;
        x = (ls * (x / m)) + (x % m) - bs;
        while(x < m)
        {
            read_byte(&byte, metadata->reader_ptr);
            x = (x << 8) + byte;
        }
    }
}