#include "block.h"

void process_block(FILE * input_file, file_header_t * header, coding_signature_t signature, output_block_t * out_block)
{
    uint32_t size;
    uint32_t cumal = 0;
    uint32_t symbol;
    uint32_t nu_offset;
    uint32_t no_unique = 0;
    uint64_t state, ls, bs, Is, m, B = 32, V;
    out_block->pre_size = 0;
    out_block->content_size = 0;
    out_block->post_size = 0;
    size = fread(header->data, sizeof(uint32_t), BLOCK_SIZE, input_file);
    if(signature.header == HEADER_BLOCK)
    {
        //clear the header
        for(uint i=0; i<=header->max; i++)
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
        for(uint i=0; i<=header->max; i++)
        {
            if(header->freq[i]){
                out_block->pre[out_block->pre_size++] = i;
                header->cumalative_freq[i] = cumal;
                cumal += header->freq[i];
                no_unique++;
            }
        }
        for(uint i=0; i<=header->max; i++)
        {
            if(header->freq[i]){
                out_block->pre[out_block->pre_size++] = header->freq[i];
            }
        }
        out_block->pre[nu_offset] = no_unique;
    }


    state = size;
    m = size;
    for(int i=size-1; i>=0; i--)
    {
        symbol = header->data[i];
        ls = header->freq[symbol];
        bs = header->cumalative_freq[symbol];
        Is = (ls << B) - 1;
        while(state > Is){
            out_block->content[out_block->content_size++] = state % (1 << B);
            state = state >> B;
        }
        state = m * (state / ls) + bs + (state % ls);
    }
    out_block->pre[out_block->pre_size++] = state >> 32;
    V = state % 4294967296;
    out_block->pre[out_block->pre_size++] = V;
    out_block->pre[out_block->pre_size++] = out_block->content_size;

}

void output_block(struct writer * my_writer, output_block_t * block)
{
    if(block->pre_size){
        write_bytes(block->pre, block->pre_size << 2, my_writer);
    }
    if(block->content_size){
        write_bytes(block->content, block->content_size << 2, my_writer);
    }
    if(block->post_size){
        write_bytes(block->post, block->post_size << 2, my_writer);
    }
}

void read_block(struct reader * my_reader, file_header_t * header, coding_signature_t signature, data_block_t * block)
{
    size_t ignore;
    uint32_t cumal = 0;
    uint32_t sym, usym;
    uint32_t * symbol = NULL;
    uint32_t * freq = NULL;
    uint32_t * sym_lookup = NULL;
    uint64_t state, ls, bs, m, Is, B = 32;
    uint32_t S, content_size;
    uint ind = 0;
    uint32_t read=0;
    if(signature.header == HEADER_BLOCK)
    {
        for(uint i=0; i<header->max; i++)
        {
            header->freq[i] = 0;
            header->cumalative_freq[i] = 0;
        }
        read_uint32_t(&(header->symbols), my_reader);
        sym_lookup = mymalloc(sizeof(uint32_t) * (header->symbols));
        read_uint32_t(&usym, my_reader);
        header->unique_symbols = usym;
        symbol = mymalloc(sizeof(uint32_t) * usym);
        freq = mymalloc(sizeof(uint32_t) * usym);
        read_bytes(symbol, sizeof(uint32_t) * usym, my_reader);
        read_bytes(freq, sizeof(uint32_t) * usym, my_reader);

        for(int i = 0; i < usym; i++)
        {
            header->cumalative_freq[symbol[i]] = cumal;
            header->freq[symbol[i]] = freq[i];
            for(uint j = 0; j < freq[i]; j++){
                sym_lookup[ind++] = symbol[i];
            }
            cumal += freq[i];
            header->max = symbol[i];
        }
    }
    block->size = 0;
    m = header->symbols;
    read_uint32_t(&S, my_reader);
    state = S;
    read_uint32_t(&S, my_reader);
    state = state << 32;
    state = state + S;
    read_uint32_t(&content_size, my_reader);
    read_bytes(header->data, sizeof(uint32_t) * content_size, my_reader);
    for(uint i=0; i<m; i++)
    {
        S = sym_lookup[state % m];
        ls = header->freq[S];
        bs = header->cumalative_freq[S];
        block->data[block->size++] = S;
        state = ls * (state / m) + (state % m) - bs;
        while(state < m && (content_size-read) > 0){
            read++;
            state = (state << B) + header->data[content_size-read];
        }
    }
    if(signature.header == HEADER_BLOCK)
    {
        myfree(sym_lookup);
        sym_lookup = NULL;
        myfree(symbol);
        symbol = NULL;
        myfree(freq);
        freq = NULL;
    }
}

void output_to_file(FILE * output_file, data_block_t * data)
{
    fwrite(data->data, sizeof(uint32_t), data->size, output_file);
}
