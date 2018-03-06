#include "block.h"

void process_block(FILE * input_file, file_header_t * header, coding_signature_t signature, output_block_t * out_block)
{
    uint32_t size;
    out_block->pre_size = 0;
    out_block->content_size = 0;
    out_block->post_size = 0;
    size = fread(out_block->content, sizeof(uint32_t), BLOCK_SIZE, input_file);
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
    //sleep(1);

}
