#include "writer.h"
#include "reader.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef PRELUDE_COMP_CODE
#define PRELUDE_COMP_CODE
#define PRELUDE_BUFFER_SIZE 1048576
#define LOG_SIZE 524288
struct prelude_code_data
 {
    struct writer * writer_ptr;
    struct bit_writer * bit_writer_ptr;
    struct reader * reader_ptr;
    struct bit_reader * bit_reader_ptr;
    uint64_t state;
    uint64_t lo;
    uint64_t hi;
    uint64_t buffer[PRELUDE_BUFFER_SIZE];
    size_t index;
 };
 struct prelude_functions
 {
    void (*func_encode)(struct prelude_code_data *,uint64_t);
    void (*func_flush)(struct prelude_code_data *);
    uint64_t (*func_decode)(struct prelude_code_data *);
 };
struct prelude_code_data * prepare_metadata(struct reader * reader_ptr, struct writer * writer_ptr, uint64_t initial_state);
// ---------------
// Variable byte
// ---------------
void vbyte_encode(struct prelude_code_data * metadata, uint64_t value);
void vbyte_flush(struct prelude_code_data * metadata);
uint64_t vbyte_decode(struct prelude_code_data * metadata);
// ---------------
// Elias
// ---------------
void elias_encode(struct prelude_code_data * metadata, uint64_t value);
void elias_flush(struct prelude_code_data * metadata);
uint64_t elias_decode(struct prelude_code_data * metadata);
// ---------------
// Golomb
// ---------------
void golomb_encode(struct prelude_code_data * metadata, uint64_t value);
void golomb_flush(struct prelude_code_data * metadata);
uint64_t golomb_decode(struct prelude_code_data * metadata);
// ---------------
// Two-pass
// ---------------
void two_pass_store(struct prelude_code_data * metadata, uint64_t value);
// ---------------
// Interpolative
// ---------------
void interp_encode(struct prelude_code_data * metadata, uint64_t value);
void interp_flush(struct prelude_code_data * metadata);
uint64_t interp_decode(struct prelude_code_data * metadata);
// ---------------
// ANS coder
// ---------------
void ans_encode(struct prelude_code_data * metadata, uint64_t value);
void ans_flush(struct prelude_code_data * metadata);
uint64_t ans_decode(struct prelude_code_data * metadata);
// ---------------
// ANS elias coder
// ---------------
void ans_elias_encode(struct prelude_code_data * metadata, uint64_t value);
void ans_elias_flush(struct prelude_code_data * metadata);
uint64_t ans_elias_decode(struct prelude_code_data * metadata);
void get_ans_elias_data(struct prelude_code_data * metadata);
// void %SCHEME%_encode(struct prelude_code_data * metadata, uint64_t value);
// void %SCHEME%_flush(struct prelude_code_data * metadata);
// uint64_t %SCHEME%_decode(struct prelude_code_data * metadata);
// just replace SCHEME with the scheme to add and follow the same format this will allow the modular application.
uint64_t flog2(uint64_t value);
void get_ans_data(struct prelude_code_data * metadata);

#endif