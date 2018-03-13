#include "block.h"

void process_block(FILE * input_file, struct writer * my_writer, file_header_t * header, coding_signature_t signature, output_block_t * out_block)
{
    uint32_t size;
    uint32_t cumal = 0;
    uint32_t symbol;
    uint32_t no_unique = 0;
    uint64_t state, ls, bs, Is, m, B = 32;
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
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
        elias_encode(metadata, size);
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
                header->cumalative_freq[i] = cumal;
                cumal += header->freq[i];
                no_unique++;
            }
        }
        elias_encode(metadata, no_unique);
        symbol = 0;
        for(uint i=0; i<=header->max; i++)
        {
            if(header->freq[i]){
                elias_encode(metadata, i - symbol);
                symbol = i;
                elias_encode(metadata, header->freq[i]);
            }
        }
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
    elias_encode(metadata, out_block->content_size);
    elias_flush(metadata);
    free_metadata(metadata);
    write_uint64_t(state, my_writer);

}

void output_block(struct writer * my_writer, output_block_t * block)
{
    if(block->content_size){
        write_bytes((unsigned char *)block->content, block->content_size << 2, my_writer);
    }
    if(block->post_size){
        write_bytes((unsigned char *)block->post, block->post_size << 2, my_writer);
    }
}

void read_block(struct reader * my_reader, file_header_t * header, coding_signature_t signature, data_block_t * block)
{
    uint32_t cumal = 0;
    uint32_t * symbol = NULL;
    uint32_t * freq = NULL;
    uint32_t * sym_lookup = NULL;
    uint64_t state, ls, bs, m, B = 32;
    uint32_t S, content_size;
    uint ind = 0;
    uint32_t read=0;
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    if(signature.header == HEADER_BLOCK)
    {
        for(uint i=0; i<header->max; i++)
        {
            header->freq[i] = 0;
            header->cumalative_freq[i] = 0;
        }
        header->symbols = elias_decode(metadata);
        sym_lookup = mymalloc(sizeof(uint32_t) * (header->symbols));
        header->unique_symbols = elias_decode(metadata);
        symbol = mymalloc(sizeof(uint32_t) * header->unique_symbols);
        freq = mymalloc(sizeof(uint32_t) * header->unique_symbols);
        S = 0;
        for(uint i=0; i<header->unique_symbols; i++){
            symbol[i] = elias_decode(metadata) + S;
            S = symbol[i];
            freq[i] = elias_decode(metadata);
        }
        for(int i = 0; i < header->unique_symbols; i++)
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
    content_size = elias_decode(metadata);
    state = elias_decode(metadata);
    read_uint64_t(&state, my_reader);
    read_bytes((unsigned char *)header->data, sizeof(uint32_t) * content_size, my_reader);
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
    free_metadata(metadata);
}

void output_to_file(FILE * output_file, data_block_t * data)
{
    fwrite(data->data, sizeof(uint32_t), data->size, output_file);
}
