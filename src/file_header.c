#include "file_header.h"

void preprocess_file(FILE * input_file, coding_signature_t signature, file_header_t * header)
{
    uint32_t no_blocks = 0;
    size_t prev = 1;
    uint32_t * block = mymalloc(sizeof(uint32_t) * BLOCK_SIZE);
    while(prev > 0)
    {
        prev = fread (block, sizeof(uint32_t), BLOCK_SIZE, input_file);
        if(prev > 0) no_blocks++;
    }
    myfree(block);
    block = NULL;
    header->no_blocks = no_blocks;
    fseek(input_file, 0, SEEK_SET);
}

void output_file_header(struct writer * my_writer, file_header_t * header, coding_signature_t signature)
{
    write_uint32_t(MAGIC, my_writer);
    write_uint32_t(signature.symbol, my_writer);
    write_uint32_t(signature.header, my_writer);
    write_uint32_t(signature.ans, my_writer);
    write_uint32_t(header->no_blocks, my_writer);
}

void read_file_header(struct reader * my_reader, coding_signature_t * signature, file_header_t * header)
{
    uint32_t magic = 0;
        read_uint32_t(&magic, my_reader);

    if(magic != MAGIC){
        fprintf(stderr, "This file doesn't appear to have been encoded properly\n");
        exit(-1);
    }
    read_uint32_t(&((*signature).symbol), my_reader);
    read_uint32_t(&((*signature).header), my_reader);
    read_uint32_t(&((*signature).ans), my_reader);
    read_uint32_t(&(header->no_blocks), my_reader);
}