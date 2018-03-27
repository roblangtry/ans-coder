#include "prelude_code.h"

void free_metadata(struct prelude_code_data * metadata)
{
    free_bit_writer(metadata->bit_writer_ptr);
    free_bit_reader(metadata->bit_reader_ptr);
    FREE(metadata);
}
struct prelude_code_data * prepare_metadata(struct reader * reader_ptr, struct writer * writer_ptr, uint64_t initial_state){
    struct prelude_code_data * metadata = (struct prelude_code_data *)mymalloc(sizeof(struct prelude_code_data));
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
// Unary
// ---------------
void unary_encode(struct prelude_code_data * metadata, uint64_t value){
    while(value > 1){
        write_bits(1, 1, metadata->bit_writer_ptr);
        value--;
    }
    write_bits(0, 1, metadata->bit_writer_ptr);
}
void unary_flush(struct prelude_code_data * metadata){
    flush_bit_writer(metadata->bit_writer_ptr);
}
uint64_t unary_decode(struct prelude_code_data * metadata){
    uint64_t value = 0;
    unsigned char read = 1;
    while(read == 1){
        read = read_bits(1, metadata->bit_reader_ptr);
        value++;
    }
    return value;
}
// ---------------
// Elias
// ---------------
void elias_encode(struct prelude_code_data * metadata, uint64_t value){
    uint64_t z = loggy(value);
    unary_encode(metadata, z+1);
    write_bits(value, z, metadata->bit_writer_ptr);
}
void elias_flush(struct prelude_code_data * metadata){
    flush_bit_writer(metadata->bit_writer_ptr);
}
uint64_t elias_decode(struct prelude_code_data * metadata){
    uint64_t z = unary_decode(metadata) - 1;
    return read_bits(z, metadata->bit_reader_ptr);
}
// ---------------
// Delta
// ---------------
void delta_encode(struct prelude_code_data * metadata, uint64_t value){
    uint64_t z = loggy(value);
    elias_encode(metadata, z+1);
    write_bits(value, z, metadata->bit_writer_ptr);
}
void delta_flush(struct prelude_code_data * metadata){
    flush_bit_writer(metadata->bit_writer_ptr);
}
uint64_t delta_decode(struct prelude_code_data * metadata){
    uint64_t z = elias_decode(metadata) - 1;
    return read_bits(z, metadata->bit_reader_ptr);
}
// // ---------------
// // Golomb
// // ---------------
// void golomb_encode(struct prelude_code_data * metadata, uint64_t value)
// {

// }
// void golomb_flush(struct prelude_code_data * metadata)
// {

// }
// uint64_t golomb_decode(struct prelude_code_data * metadata)
// {
//     return 0;
// }

// // ---------------
// // Two-pass
// // ---------------
// void two_pass_store(struct prelude_code_data * metadata, uint64_t value)
// {
//     if(metadata->index == 0){
//         metadata->lo = value;
//         metadata->hi = value;
//     }
//     if(metadata->lo > value) metadata->lo = value;
//     if(metadata->hi < value) metadata->hi = value;
//     metadata->buffer[metadata->index] = value;
//     metadata->index += 1;
// }
// // ---------------
// // Interpolative
// // ---------------
// void interp_encode(struct prelude_code_data * metadata, uint64_t value)
// {
//     two_pass_store(metadata, value);
// }
// void interp_flush(struct prelude_code_data * metadata)
// {

// }
// uint64_t interp_decode(struct prelude_code_data * metadata)
// {
//     return 0;
// }
// // ---------------
// // ANS coder
// // ---------------
// void ans_encode(struct prelude_code_data * metadata, uint64_t value)
// {
//     two_pass_store(metadata, value);
// }
// void ans_flush(struct prelude_code_data * metadata)
// {
//     size_t output_mod = 1;
//     size_t output_index = 0;
//     size_t meta_index = 0;
//     size_t size = PRELUDE_BUFFER_SIZE * output_mod * sizeof(unsigned char);
//     unsigned char * output = (unsigned char *)mymalloc(size);
//     uint64_t b = 256;
//     uint64_t m = metadata->hi - metadata->lo + 1;
//     uint64_t x = m;
//     uint64_t ls;
//     uint64_t bs;
//     uint64_t s;
//     unsigned char byte;
//     while(meta_index != metadata->index)
//     {
//         if((size - 10) <= output_index)
//         {
//             output_mod += 1;
//             size = PRELUDE_BUFFER_SIZE * output_mod * sizeof(unsigned char);
//             if(realloc(output, size) == NULL) return;
//         }
//         s = metadata->buffer[metadata->index - meta_index - 1];
//         ls = 1;
//         bs = s - metadata->lo;
//         while(x > ((ls * b)-1))
//         {
//             byte = x % b;
//             output[output_index] = byte;
//             output_index += 1;
//             x = x / b;
//         }
//         x = (m * (x / ls)) + bs + (x % ls);
//         meta_index += 1;
//     }
//     vbyte_encode(metadata, metadata->lo);
//     vbyte_encode(metadata, metadata->hi);
//     vbyte_encode(metadata, x);
//     vbyte_encode(metadata, metadata->index);
//     vbyte_flush(metadata);
//     meta_index = 0;
//     while(meta_index < output_index){
//         byte = output[output_index - meta_index - 1];
//         write_byte(byte, metadata->writer_ptr);
//         meta_index += 1;
//     }
// }
// uint64_t ans_decode(struct prelude_code_data * metadata)
// {
//     uint64_t value;
//     if(metadata->hi == 0 && metadata->lo == 0) get_ans_data(metadata);
//     value = metadata->buffer[metadata->index];
//     metadata->index += 1;
//     return value;
// }
// void get_ans_data(struct prelude_code_data * metadata)
// {
//     uint64_t lo = vbyte_decode(metadata);
//     uint64_t hi = vbyte_decode(metadata);
//     uint64_t x = vbyte_decode(metadata);
//     uint64_t m = hi - lo + 1;
//     uint64_t s;
//     uint64_t ls;
//     uint64_t bs;
//     size_t num = vbyte_decode(metadata);
//     size_t i = 0;
//     unsigned char byte;
//     metadata->state = num;
//     metadata->index = 0;
//     metadata->lo = lo;
//     metadata->hi = hi;
//     while(i < num)
//     {
//         s = x % m + lo;
//         ls = 1;
//         bs = s - lo;
//         metadata->buffer[i] = s;
//         i += 1;
//         x = (ls * (x / m)) + (x % m) - bs;
//         while(x < m)
//         {
//             read_byte(&byte, metadata->reader_ptr);
//             x = (x << 8) + byte;
//         }
//     }
// }

// // ---------------
// // ANS coder 2
// // ---------------
// void ans_elias_encode(struct prelude_code_data * metadata, uint64_t value)
// {
//     two_pass_store(metadata, value);
// }
// void ans_elias_flush(struct prelude_code_data * metadata)
// {
//     size_t output_mod = 1;
//     size_t output_index = 0;
//     size_t meta_index = 0;
//     size_t size = PRELUDE_BUFFER_SIZE * output_mod * sizeof(unsigned char);
//     unsigned char * output = (unsigned char *)mymalloc(size);
//     uint64_t b = 256;
//     uint64_t * log_lookup = calloc(metadata->hi + 1, sizeof(uint64_t));
//     log_lookup[metadata->hi] = flog2(metadata->hi);
//     uint64_t max = log_lookup[metadata->hi];
//     uint64_t m = 2 << max;
//     m = m << max;
//     uint64_t x = m;
//     uint64_t ls;
//     uint64_t bs;
//     uint64_t s;
//     unsigned char byte;
//     while(meta_index != metadata->index)
//     {
//         if((size - 10) <= output_index)
//         {
//             output_mod += 1;
//             size = PRELUDE_BUFFER_SIZE * output_mod * sizeof(unsigned char);
//             if(realloc(output, size) == NULL) return;
//         }
//         s = metadata->buffer[metadata->index - meta_index - 1];
//         if (log_lookup[s] == 0) log_lookup[s] = flog2(s);
//         ls = 1 << (max - log_lookup[s]);
//         ls = ls << (max - log_lookup[s]);
//         bs = m - (m >> log_lookup[s]);
//         bs += ((s - (1 << log_lookup[s])) * ls);
//         while(x > ((ls * b)-1))
//         {
//             byte = x % b;
//             output[output_index] = byte;
//             output_index += 1;
//             x = x / b;
//         }
//         x = (m * (x / ls)) + bs + (x % ls);
//         meta_index += 1;
//     }
//     vbyte_encode(metadata, metadata->hi);
//     vbyte_encode(metadata, x);
//     vbyte_encode(metadata, metadata->index);
//     vbyte_flush(metadata);
//     meta_index = 0;
//     while(meta_index < output_index){
//         byte = output[output_index - meta_index - 1];
//         write_byte(byte, metadata->writer_ptr);
//         meta_index += 1;
//     }
// }
// uint64_t ans_elias_decode(struct prelude_code_data * metadata)
// {
//     uint64_t value;
//     if(metadata->hi == 0 && metadata->lo == 0) get_ans_elias_data(metadata);
//     value = metadata->buffer[metadata->index];
//     metadata->index += 1;
//     return value;
// }
// void get_ans_elias_data(struct prelude_code_data * metadata)
// {
//     uint64_t hi = vbyte_decode(metadata);
//     uint64_t x = vbyte_decode(metadata);
//     uint64_t m;
//     uint64_t s;
//     uint64_t ls;
//     uint64_t bs;
//     uint64_t c;
//     uint64_t q;
//     uint64_t r;
//     uint64_t v3;
//     uint64_t v4;
//     uint64_t v5;
//     size_t num = vbyte_decode(metadata);
//     size_t i = 0;
//     unsigned char byte;
//     metadata->state = num;
//     metadata->index = 0;
//     metadata->hi = hi;
//     uint64_t max = flog2(metadata->hi);
//     m = 2 << max;
//     m = m << max;
//     uint64_t * log_lookup = calloc(SYMBOL_MAP_SIZE + 1, sizeof(uint64_t));
//     while(i < num)
//     {
//         if (log_lookup[m - (x % m) - 1] == 0) log_lookup[m - (x % m) - 1] = flog2(m - (x % m) - 1);
//         if (log_lookup[m] == 0) log_lookup[m] = flog2(m);
//         c = log_lookup[m] - log_lookup[m - (x % m) - 1] - 1;
//         q = 1 << c;
//         v3 = x % m;
//         v4 = m >> (c + 1);
//         if(v4 == 0) v5 = 0;
//         else v5 = v3 % v4;
//         if ((v4 / q) == 0) r = 0;
//         else r = v5 / (v4 / q);
//         s = q + r;
//         if (log_lookup[s] == 0) log_lookup[s] = flog2(s);
//         ls = 1 << ((max - log_lookup[s]));
//         ls = ls << ((max - log_lookup[s]));
//         bs = m - (m >> log_lookup[s]);
//         bs += ((s - (1 << log_lookup[s])) * ls);
//         metadata->buffer[i] = s;
//         i += 1;
//         x = (ls * (x / m)) + (x % m) - bs;
//         while(x < m)
//         {
//             read_byte(&byte, metadata->reader_ptr);
//             x = (x << 8) + byte;
//         }
//     }
// }

uint64_t flog2(uint64_t value)
{
    return floor(log(value) / log(2));
}

uint64_t loggy(uint64_t value)
{
    uint64_t i = 1;
    while(value>>=1) i++;
    return i;
}