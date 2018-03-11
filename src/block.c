#include "block.h"

void process_block(FILE * input_file, file_header_t * header, coding_signature_t signature, output_block_t * out_block)
{
    uint32_t size;
    uint32_t cumal = 0;
    uint32_t symbol;
    uint32_t nu_offset;
    uint32_t no_unique = 0;
    out_block->pre_size = 0;
    out_block->content_size = 0;
    out_block->post_size = 0;
    size = fread(header->data, sizeof(uint32_t), BLOCK_SIZE, input_file);
    if(signature.header == HEADER_BLOCK)
    {
        //clear the header
        for(uint i=0; i<header->max; i++)
        {
            header->freq[i] = 0;
            header->cumalative_freq[i] = 0;
        }
        header->max = 0;
        //read the block
        out_block->pre[out_block->pre_size++] = size;
        nu_offset = out_block->pre_size;
        out_block->pre_size++;
        for(uint i=0; i<size; i++)
        {
            symbol = header->data[i];
            header->freq[symbol]++;
            if(symbol > header->max) header->max = symbol;
        }
        //calculate cumalative frequency
        for(uint i=0; i<header->max; i++)
        {
            if(header->freq[i]){
                out_block->pre[out_block->pre_size++] = i;
                header->cumalative_freq[i] = cumal;
                cumal += header->freq[i];
                no_unique++;
            }
        }
        for(uint i=0; i<header->max; i++)
        {
            if(header->freq[i]){
                out_block->pre[out_block->pre_size++] = header->freq[i];
            }
        }
    }
    for(uint i=0; i<size; i++)
    {
        out_block->content[i] = header->data[i];
    }
    out_block->pre[nu_offset] = no_unique;
    out_block->content_size = size;
}

void output_block(FILE * output_file, output_block_t * block)
{
    if(block->pre_size){
        fwrite(block->pre, sizeof(uint32_t), block->pre_size, output_file);
    }
    if(block->content_size){
        fwrite(block->content, sizeof(uint32_t), block->content_size, output_file);
    }
    if(block->post_size){
        fwrite(block->post, sizeof(uint32_t), block->post_size, output_file);
    }
}

void read_block(FILE * input_file, file_header_t * header, coding_signature_t signature, data_block_t * block)
{
    size_t size;
    size_t ignore;
    uint32_t cumal = 0;
    uint32_t sym, usym;
    uint32_t * symbol;
    uint32_t * freq;
    if(signature.header == HEADER_BLOCK)
    {
        for(uint i=0; i<header->max; i++)
        {
            header->freq[i] = 0;
            header->cumalative_freq[i] = 0;
        }
        fread(&(header->symbols), sizeof(uint32_t), 1, input_file);
        fread(&usym, sizeof(uint32_t), 1, input_file);
        header->unique_symbols = usym;
        symbol = mymalloc(sizeof(uint32_t) * usym);
        freq = mymalloc(sizeof(uint32_t) * usym);
        fread(symbol, sizeof(uint32_t), usym, input_file);
        fread(freq, sizeof(uint32_t), usym, input_file);
        for(int i = 0; i < usym; i++)
        {
            header->cumalative_freq[symbol[i]] = cumal;
            header->freq[symbol[i]] = freq[i];
            cumal += freq[i];
            header->max = symbol[i];
        }
    }
    block->size = header->symbols;
    ignore = fread(block->data, sizeof(uint32_t), block->size, input_file);
    myfree(symbol);
    myfree(freq);
}

void output_to_file(FILE * output_file, data_block_t * data)
{
    fwrite(data->data, sizeof(uint32_t), data->size, output_file);
}
