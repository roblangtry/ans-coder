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

void read_file_header(FILE * input_file, coding_signature_t * signature, file_header_t * header)
{
    uint32_t magic = 0;
    if(fread(&magic, sizeof(uint32_t), 1, input_file) == 0)
    {
        fprintf(stderr, "Read Failure\n");
        exit(-1);
    }
    if(magic != MAGIC){
        fprintf(stderr, "This file doesn't appear to have been encoded properly\n");
        exit(-1);
    }
    if(fread(&((*signature).symbol), sizeof(uint32_t), 1, input_file) == 0)
    {
        fprintf(stderr, "Read Failure\n");
        exit(-1);
    }
    if(fread(&((*signature).header), sizeof(uint32_t), 1, input_file) == 0)
    {
        fprintf(stderr, "Read Failure\n");
        exit(-1);
    }
    if(fread(&((*signature).ans), sizeof(uint32_t), 1, input_file) == 0)
    {
        fprintf(stderr, "Read Failure\n");
        exit(-1);
    }
    if(fread(&(header->no_blocks), sizeof(uint32_t), 1, input_file) == 0)
    {
        fprintf(stderr, "Read Failure\n");
        exit(-1);
    }
}