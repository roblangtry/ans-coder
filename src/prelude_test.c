#include "prelude_test.h"
void low_order_simple_test(
    void (*func_encode)(struct prelude_code_data *,uint64_t),
    void (*func_flush)(struct prelude_code_data *),
    uint64_t (*func_decode)(struct prelude_code_data *),
    char * encoding)
{
    struct timespec start, end;
    int i = 0;
    uint64_t val;
    uint64_t delta_us;
    FILE * input_file;
    FILE * output_file;
    output_file = fopen("test.stack", "w");
    struct prelude_code_data * metadata = prepare_metadata(NULL, initialise_writer(output_file), 0);
    printf("%s,LOST,", encoding);
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    while(i < TEST_SIZE){
        func_encode(metadata, 1 + (i % 128));
        i++;
    }
    func_flush(metadata);
    flush_writer(metadata->writer_ptr);
    fclose(output_file);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("%ld,", delta_us);
    input_file = fopen("test.stack", "r");
    metadata = prepare_metadata(initialise_reader(input_file), NULL, 0);
    i = 0;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    while(i < TEST_SIZE){
        val = func_decode(metadata);
        if(val != (1 + (i % 128))) printf("(%s) VIOLATION i=%d\n (exp= %d | got= %ld )\n", encoding, i, 1 + (i % 128), val);
        i++;
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("%ld,", delta_us);
    printf("%ld,%d\n", ftell(input_file), TEST_SIZE);
    fclose(input_file);
}

void high_order_simple_test(
    void (*func_encode)(struct prelude_code_data *,uint64_t),
    void (*func_flush)(struct prelude_code_data *),
    uint64_t (*func_decode)(struct prelude_code_data *),
    char * encoding)
{
    struct timespec start, end;
    int i = 0;
    uint64_t val;
    uint64_t delta_us;
    FILE * input_file;
    FILE * output_file;
    output_file = fopen("test.stack", "w");
    struct prelude_code_data * metadata = prepare_metadata(NULL, initialise_writer(output_file), 0);
    printf("%s,HOST,", encoding);
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    while(i < TEST_SIZE){
        func_encode(metadata, 10000000 + (i % 2000));
        i++;
    }
    func_flush(metadata);
    flush_writer(metadata->writer_ptr);
    fclose(output_file);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("%ld,", delta_us);
    input_file = fopen("test.stack", "r");
    metadata = prepare_metadata(initialise_reader(input_file), NULL, 0);
    i = 0;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    while(i < TEST_SIZE){
        val = func_decode(metadata);
        if(val != 10000000 + (i % 2000)) printf("(%s) VIOLATION i=%d\n (exp= %d | got= %ld )\n", encoding, i, 10000000 + (i % 2000), val);
        i++;
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("%ld,", delta_us);
    printf("%ld,%d\n", ftell(input_file), TEST_SIZE);
    fclose(input_file);
}

void complex_order_test(
    void (*func_encode)(struct prelude_code_data *,uint64_t),
    void (*func_flush)(struct prelude_code_data *),
    uint64_t (*func_decode)(struct prelude_code_data *),
    char * encoding)
{
    struct timespec start, end;
    int i = 0;
    uint64_t val;
    uint64_t sym;
    uint64_t delta_us;
    FILE * input_file;
    FILE * output_file;
    output_file = fopen("test.stack", "w");
    struct prelude_code_data * metadata = prepare_metadata(NULL, initialise_writer(output_file), 0);
    printf("%s,COT,", encoding);
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    while(i < TEST_SIZE){
        if(i % 2 == 0) sym = (i % 20) + 1;
        else sym = 1000000 + i;
        func_encode(metadata, sym);
        i++;
    }
    func_flush(metadata);
    flush_writer(metadata->writer_ptr);
    fclose(output_file);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("%ld,", delta_us);
    input_file = fopen("test.stack", "r");
    metadata = prepare_metadata(initialise_reader(input_file), NULL, 0);
    i = 0;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    while(i < TEST_SIZE){
        val = func_decode(metadata);
        if(i % 2 == 0) sym = (i % 20) + 1;
        else sym = 1000000 + i;
        if(val != sym) printf("(%s) VIOLATION i=%d\n (exp= %d | got= %ld )\n", encoding, i, sym, val);
        i++;
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("%ld,", delta_us);
    printf("%ld,%d\n", ftell(input_file), TEST_SIZE);
    fclose(input_file);
}

void test_battery(
    void (*func_encode)(struct prelude_code_data *,uint64_t),
    void (*func_flush)(struct prelude_code_data *),
    uint64_t (*func_decode)(struct prelude_code_data *),
    char * encoding)
{
    int i = 0;
    while (i < TEST_REPS){
        complex_order_test(func_encode,func_flush,func_decode,encoding);
        low_order_simple_test(func_encode,func_flush,func_decode,encoding);
        high_order_simple_test(func_encode,func_flush,func_decode,encoding);
        i++;
    }
}

int main(){
    printf("encoding,test,compression_time,decompression_time,compressed_size,no_symbols\n");
    test_battery(ans_encode,ans_flush,ans_decode,"ANS");
    test_battery(elias_encode,elias_flush,elias_decode,"ELIAS");
    test_battery(ans_elias_encode,ans_elias_flush,ans_elias_decode,"ANS_ELIAS");
    test_battery(vbyte_encode,vbyte_flush,vbyte_decode,"VBYTE");
}