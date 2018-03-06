#include "block.h"

void process_block(FILE * input_file, file_header_t * header, coding_signature_t signature, output_block_t * out_block)
{
    uint32_t size;
    uint32_t cumal = 0;
    uint32_t symbol;
    out_block->pre_size = 0;
    out_block->content_size = 0;
    out_block->post_size = 0;
    size = fread(header->data, sizeof(uint32_t), BLOCK_SIZE, input_file);
    if(signature.header == HEADER_BLOCK)
    {
        for(uint i=0; i<header->max; i++)
        {
            header->freq[i] = 0;
            header->cumalative_freq[i] = 0;
        }
        header->max = 0;
        for(uint i=0; i<size; i++)
        {
            symbol = out_block->content[i];
            header->freq[symbol]++;
            if(symbol > header->max) header->max = symbol;
        }
        for(uint i=0; i<header->max; i++)
        {
            header->cumalative_freq[i] = cumal;
            cumal += header->freq[i];
        }
    }
    for(uint i=0; i<size; i++)
    {
        out_block->content[i] = header->data[i];
    }
    out_block->content_size = size;
    out_block->pre[0] = size;
    out_block->pre_size = 1;
}

void output_block(FILE * output_file, output_block_t * block)
{
    if(block->pre_size)
        fwrite(block->pre, sizeof(uint32_t), block->pre_size, output_file);
    if(block->content_size)
        fwrite(block->content, sizeof(uint32_t), block->content_size, output_file);
    if(block->post_size)
        fwrite(block->post, sizeof(uint32_t), block->post_size, output_file);
}

void read_block(FILE * input_file, file_header_t * header, coding_signature_t signature, data_block_t * block)
{
    size_t size;
    size_t ignore;
    ignore = fread(&(block->size), sizeof(uint32_t), 1, input_file);
    ignore = fread(block->data, sizeof(uint32_t), block->size, input_file);
}

void output_to_file(FILE * output_file, data_block_t * data)
{
    fwrite(data->data, sizeof(uint32_t), data->size, output_file);
}
