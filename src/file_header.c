#include "file_header.h"

encoding_file_t preprocess_file(FILE * input_file, coding_signature_t signature)
{
    encoding_file_t file;
    uint32_t no_blocks = 0;
    size_t prev = 1;
    size_t i = 0;
    uint32_t * block = mymalloc(sizeof(uint32_t) * BLOCK_SIZE);
    while(prev > 0)
    {
        prev = fread (block, sizeof(uint32_t), BLOCK_SIZE, input_file);
        if(prev > 0) no_blocks++;
    }
    myfree(block);
    file.header.no_blocks = no_blocks;
    fseek(input_file, 0, SEEK_SET);
    return file;
}

void output_file_header(FILE * output_file, encoding_file_t file, coding_signature_t signature)
{
    uint32_t magic = MAGIC;
    fwrite(&magic, sizeof(uint32_t), 1, output_file);
    fwrite(&signature.symbol, sizeof(uint32_t), 1, output_file);
    fwrite(&signature.header, sizeof(uint32_t), 1, output_file);
    fwrite(&signature.ans, sizeof(uint32_t), 1, output_file);
    fwrite(&file.header.no_blocks, sizeof(uint32_t), 1, output_file);
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