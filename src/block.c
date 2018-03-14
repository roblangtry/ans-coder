#include "block.h"

void process_block(FILE * input_file, struct writer * my_writer, file_header_t * header, coding_signature_t signature, output_block_t * out_block)
{
    uint32_t size;
    uint32_t cumal = 0;
    uint32_t symbol;
    uint32_t no_unique = 0;
    uint64_t state, ls, bs, Is, m, B = 32;
    unsigned char * msb_link = (unsigned char *)out_block->post;
    uint32_t msb_ind = 0;
    unsigned char vbyte, byte;
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    out_block->pre_size = 0;
    out_block->content_size = 0;
    out_block->post_size = 0;
    size = fread(header->data, sizeof(uint32_t), BLOCK_SIZE, input_file);
    if(signature.header == HEADER_BLOCK)
    {

        header->symbols = size;
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
            if(signature.symbol == SYMBOL_DIRECT) symbol = header->data[i];
            else if(signature.symbol == SYMBOL_MSB) symbol = get_msb_symbol(header->data[i]);
            else exit(-1);
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
    else if(signature.header == HEADER_SINGLE)
    {
        elias_encode(metadata, size);
    }


    state = header->symbols;
    m = header->symbols;
    for(int i=size-1; i>=0; i--)
    {
        if(signature.symbol == SYMBOL_DIRECT) symbol = header->data[i];
        else if(signature.symbol == SYMBOL_MSB) symbol = get_msb_symbol(header->data[i]);
        else exit(-1);
        ls = header->freq[symbol];
        bs = header->cumalative_freq[symbol];
        Is = (ls << B) - 1;
        while(state > Is){
            out_block->content[out_block->content_size++] = state % (1 << B);
            state = state >> B;
        }
        state = m * (state / ls) + bs + (state % ls);
        if(signature.symbol == SYMBOL_MSB)
        {
            if(header->data[i] > 256 && header->data[i] <= 65536)
            {
                vbyte = header->data[i] % 256;
                byte = vbyte;
                msb_link[msb_ind++] = byte;
            }
            else if(header->data[i] > 65536 && header->data[i] <= 16777216)
            {
                vbyte = (header->data[i] >> 8) % 256;
                byte = vbyte;
                msb_link[msb_ind++] = byte;
                vbyte = header->data[i] % 256;
                byte = vbyte;
                msb_link[msb_ind++] = byte;
            }
            out_block->post_size = msb_ind / 4;
            if(msb_ind % 4) out_block->post_size++;
        }
    }
    elias_encode(metadata, out_block->content_size);
    if(signature.symbol == SYMBOL_MSB) elias_encode(metadata, out_block->post_size);
    if(signature.symbol == SYMBOL_MSB) elias_encode(metadata, msb_ind % 4);
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
    uint64_t state, ls, bs, m, B = 32;
    uint32_t S, content_size, post_size;
    uint ind = 0, msb_ind = 0, msb_offset = 0;
    uint32_t read=0, len = 0;
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    unsigned char * msb_bytes;
    unsigned char byte;
    uint i;
    if(signature.header == HEADER_BLOCK)
    {
        for(uint i=0; i<header->max; i++)
        {
            header->freq[i] = 0;
            header->cumalative_freq[i] = 0;
        }
        header->symbols = elias_decode(metadata);
        len = header->symbols;
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
                header->symbol_state[ind++] = symbol[i];
            }
            cumal += freq[i];
            header->max = symbol[i];
        }
    }
    else if(signature.header == HEADER_SINGLE)
    {
        len = elias_decode(metadata);
    }
    block->size = 0;
    m = header->symbols;
    content_size = elias_decode(metadata);
    if(signature.symbol == SYMBOL_MSB){
        post_size = elias_decode(metadata);
        msb_offset = elias_decode(metadata);
        if(msb_offset) msb_ind = ((post_size-1) << 2) + msb_offset;
        else msb_ind = post_size << 2;
        msb_bytes = mymalloc(sizeof(uint32_t) * post_size);
    }
    read_uint64_t(&state, my_reader);
    read_bytes((unsigned char *)header->data, sizeof(uint32_t) * content_size, my_reader);
    if(signature.symbol == SYMBOL_MSB) read_bytes(msb_bytes, sizeof(uint32_t) * post_size, my_reader);
    for(i=0; i<len; i++)
    {
        S = header->symbol_state[state % m];
        ls = header->freq[S];
        bs = header->cumalative_freq[S];
        block->data[block->size++] = S;
        state = ls * (state / m) + (state % m) - bs;
        while(state < m && (content_size-read) > 0){
            read++;
            state = (state << B) + header->data[content_size-read];
        }
    }
    if(signature.symbol == SYMBOL_MSB)
    {
        i = 0;
        msb_ind--;
        while(i < len)
        {

            if(block->data[i] > 256 && block->data[i] <= 512)
            {
                // printf("%u) %u[%u] (%u)\n", i, block->data[i],msb_bytes[msb_ind], ((block->data[i] - 256)<<8) + msb_bytes[msb_ind]);
                // sleep(1);
                block->data[i] -= 256;
                byte = msb_bytes[msb_ind--];
                block->data[i] = (block->data[i]<<8) + byte;
            }
            else if(block->data[i] > 512)
            {
                block->data[i] -= 512;
                byte = msb_bytes[msb_ind--];
                block->data[i] = (block->data[i]<<16) + byte;
                byte = msb_bytes[msb_ind--];
                block->data[i] = (block->data[i]) + (byte<<8);
            }
            i++;
        }
        myfree(msb_bytes);
    }
    if(signature.header == HEADER_BLOCK)
    {
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
