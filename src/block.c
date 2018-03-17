#include "block.h"

void process_block(FILE * input_file, struct writer * my_writer, file_header_t * header, coding_signature_t signature)
{
    uint32_t size;
    uint32_t cumal = 0;
    uint32_t symbol;
    uint32_t no_unique = 0;
    uint64_t state, ls, bs, Is, m, B = 32;
    unsigned char vbyte, byte;
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    int_page_t * ans_pages = get_int_page();
    char_page_t * msb_pages;
    if(signature.symbol == SYMBOL_MSB) msb_pages = get_char_page();
    size = fread(header->data, sizeof(uint32_t), BLOCK_SIZE, input_file);

    // ------------- //
    // PRE
    // ------------- //

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
            add_to_int_page(state % (1 << B), ans_pages);
            state = state >> B;
        }
        state = m * (state / ls) + bs + (state % ls);
        if(signature.symbol == SYMBOL_MSB)
        {
            if(header->data[i] > 256 && header->data[i] <= 65536)
            {
                vbyte = header->data[i] % 256;
                byte = vbyte;
                add_to_char_page(byte, msb_pages);
            }
            else if(header->data[i] > 65536 && header->data[i] <= 16777216)
            {
                vbyte = (header->data[i] >> 8) % 256;
                byte = vbyte;
                add_to_char_page(byte, msb_pages);
                vbyte = header->data[i] % 256;
                byte = vbyte;
                add_to_char_page(byte, msb_pages);
            }
        }
    }
    elias_encode(metadata, ans_pages->current_size);
    if(signature.symbol == SYMBOL_MSB) elias_encode(metadata, msb_pages->current_size);
    elias_flush(metadata);
    free_metadata(metadata);
    write_uint64_t(state, my_writer);

    // ------------- //
    // Content
    // ------------- //
    output_int_page(my_writer, ans_pages);
    // ------------- //
    // Post
    // ------------- //
    if(signature.symbol == SYMBOL_MSB) output_char_page(my_writer, msb_pages);
    free_int_page(ans_pages);
    if(signature.symbol == SYMBOL_MSB) free_char_page(msb_pages);

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
    uint64_t state, ls, bs, m, B = 32, sym_map_size;
    uint32_t S, F, content_size, post_size = 0;
    uint ind = 0;
    uint32_t read=0, len = 0;
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    unsigned char * msb_bytes = NULL;
    unsigned char byte;
    uint i;
    if(signature.header == HEADER_BLOCK)
    {
        free(header->freq);
        free(header->cumalative_freq);
        sym_map_size = SYMBOL_MAP_SIZE;
        if(signature.symbol == SYMBOL_MSB) sym_map_size = get_msb_symbol(SYMBOL_MAP_SIZE);
        header->freq = calloc(sym_map_size , sizeof(uint32_t));
        header->cumalative_freq = calloc(sym_map_size , sizeof(uint32_t));
        header->symbols = elias_decode(metadata);
        len = header->symbols;
        header->unique_symbols = elias_decode(metadata);
        // symbol = mymalloc(sizeof(uint32_t) * header->unique_symbols);
        // freq = mymalloc(sizeof(uint32_t) * header->unique_symbols);
        S = 0;
        for(uint i=0; i<header->unique_symbols; i++){
            S = elias_decode(metadata) + S;
            F = elias_decode(metadata);
            header->cumalative_freq[S] = cumal;
            header->freq[S] = F;
            for(uint j = 0; j < F; j++){
                header->symbol_state[ind++] = S;
            }
            cumal += F;
            header->max = S;
        }
    }
    else if(signature.header == HEADER_SINGLE)
    {
        len = elias_decode(metadata);
    }
    block->size = 0;
    m = header->symbols;
    content_size = elias_decode(metadata);
    if(signature.symbol == SYMBOL_MSB) post_size = elias_decode(metadata);
    read_uint64_t(&state, my_reader);
    read_bytes((unsigned char *)header->data, sizeof(uint32_t) * content_size, my_reader);
    if(signature.symbol == SYMBOL_MSB){
        msb_bytes = mymalloc(sizeof(unsigned char) * post_size);
        read_bytes(msb_bytes, sizeof(unsigned char) * post_size, my_reader);
    }
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
        post_size--;
        while(i < len)
        {

            if(block->data[i] > 256 && block->data[i] <= 512)
            {
                block->data[i] -= 256;
                byte = msb_bytes[post_size--];
                block->data[i] = (block->data[i]<<8) + byte;
            }
            else if(block->data[i] > 512)
            {
                block->data[i] -= 512;
                byte = msb_bytes[post_size--];
                block->data[i] = (block->data[i]<<16) + byte;
                byte = msb_bytes[post_size--];
                block->data[i] = (block->data[i]) + (byte<<8);
            }
            i++;
        }
        myfree(msb_bytes);
    }
    free_metadata(metadata);
}

void output_to_file(FILE * output_file, data_block_t * data)
{
    fwrite(data->data, sizeof(uint32_t), data->size, output_file);
}
