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
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    elias_encode(metadata, signature.symbol);
    elias_encode(metadata, signature.header);
    elias_encode(metadata, signature.ans);
    elias_encode(metadata, header->no_blocks);
    elias_flush(metadata);
    free_metadata(metadata);
}

void read_file_header(struct reader * my_reader, coding_signature_t * signature, file_header_t * header)
{
    uint32_t magic = 0;
        read_uint32_t(&magic, my_reader);

    if(magic != MAGIC){
        fprintf(stderr, "This file doesn't appear to have been encoded properly\n");
        exit(-1);
    }
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    (*signature).symbol = elias_decode(metadata);
    (*signature).header = elias_decode(metadata);
    (*signature).ans = elias_decode(metadata);
    header->no_blocks = elias_decode(metadata);
    free_metadata(metadata);
}