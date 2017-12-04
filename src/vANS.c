#include "vANS.h"


void vANS_encode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions)
{
    uint32_t block[BLOCK_LEN];
    size_t size;
    struct writer * my_writer = initialise_writer(output_file);
    write_meta_header(input_file, my_writer);
    size = fread(block, sizeof(uint32_t), BLOCK_LEN, input_file);
    while(size > 0){
        process_encode_block(block, size, my_writer, my_prelude_functions);
        size = fread(block, sizeof(uint32_t), BLOCK_LEN, input_file);
    }
    flush_writer(my_writer);
}

void vANS_decode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions)
{
    uint32_t no_blocks;
    uint32_t i = 0;
    struct reader * my_reader = initialise_reader(input_file);
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    no_blocks = vbyte_decode(metadata);
    while(i < no_blocks){
        process_decode_block(my_reader, output_file, my_prelude_functions);
        i++;
    }
}
