#include "block.h"

void process_block(FILE * input_file, struct writer * my_writer, file_header_t * header, coding_signature_t signature)
{
    uint32_t size;
    uint32_t symbol;
    uint32_t no_unique = 0;
    uint64_t state, ls, bs, Is, m, bits = signature.bit_factor, msb_bits = signature.msb_bit_factor;
    unsigned char vbyte, byte;
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    int_page_t * ans_pages = get_int_page();
    int_page_t * msb_pages;
    if(signature.symbol == SYMBOL_MSB) msb_pages = get_int_page();
    size = fread(header->data, sizeof(uint32_t), BLOCK_SIZE, input_file);

    // ------------- //
    // PRE
    // ------------- //

    if(signature.header == HEADER_BLOCK)
    {

        header->symbols = size;
        //clear the header
        for(uint i=0; i<=(header->max+1); i++)
        {
            header->freq[i] = 0;
        }
        header->max = 0;
        //read the block
        elias_encode(metadata, size);
        for(uint i=0; i<size; i++)
        {
            if(signature.symbol == SYMBOL_DIRECT) symbol = header->data[i];
            else if(signature.symbol == SYMBOL_MSB) symbol = get_msb_symbol(header->data[i]);
            else exit(-1);
            if(!header->freq[symbol+1]) no_unique++;
            header->freq[symbol+1]++;
            if(symbol > header->max) header->max = symbol;
        }
        //calculate cumalative frequency
        for(uint i=1; (i<=header->max+1); i++)
        {
            header->freq[i] += header->freq[i-1];
        }
        elias_encode(metadata, no_unique);
        symbol = 0;
        for(uint i=0; i<=header->max; i++)
        {
            if(header->freq[i]<header->freq[i+1]){
                elias_encode(metadata, i - symbol);
                symbol = i;
                elias_encode(metadata, header->freq[i+1] - header->freq[i]);
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
        ls = header->freq[symbol+1] - header->freq[symbol];
        bs = header->freq[symbol];
        Is = (ls << bits) - 1;
        while(state > Is){
            add_to_int_page(state % (1 << bits), ans_pages);
            state = state >> bits;
        }
        state = m * (state / ls) + bs + (state % ls);
        if(signature.symbol == SYMBOL_MSB)
        {
            if(header->data[i] > 256 && header->data[i] <= 65536)
            {
                vbyte = header->data[i] % 256;
                byte = vbyte;
                add_to_int_page(byte, msb_pages);
            }
            else if(header->data[i] > 65536 && header->data[i] <= 16777216)
            {
                vbyte = (header->data[i] >> 8) % 256;
                byte = vbyte;
                add_to_int_page(byte, msb_pages);
                vbyte = header->data[i] % 256;
                byte = vbyte;
                add_to_int_page(byte, msb_pages);
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
    output_int_page(my_writer, ans_pages, bits);
    // ------------- //
    // Post
    // ------------- //
    free_int_page(ans_pages);
    if(signature.symbol == SYMBOL_MSB){
        output_int_page(my_writer, msb_pages, msb_bits);
        free_int_page(msb_pages);
    }
}

void read_block(struct reader * my_reader, file_header_t * header, coding_signature_t signature, data_block_t * block)
{
    uint64_t state, ls, bs, m, bits = signature.bit_factor, msb_bits = signature.msb_bit_factor, sym_map_size;
    uint32_t S, F, S0 = 1, content_size, post_size = 0;
    uint ind = 0;
    uint32_t read=0, len = 0;
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    uint32_t * msb_bytes = NULL;
    unsigned char byte;
    uint i;
    if(signature.header == HEADER_BLOCK)
    {
        free(header->freq);
        sym_map_size = SYMBOL_MAP_SIZE;
        if(signature.symbol == SYMBOL_MSB) sym_map_size = get_msb_symbol(SYMBOL_MAP_SIZE);
        header->freq = calloc(sym_map_size , sizeof(uint32_t));
        header->symbols = elias_decode(metadata);
        len = header->symbols;
        header->unique_symbols = elias_decode(metadata);
        S = 0;
        for(uint i=0; i<header->unique_symbols; i++){
            S = elias_decode(metadata) + S;
            if(i==0) S0 = S;
            F = elias_decode(metadata);
            header->freq[S+1] = F;
            for(uint j = 0; j < F; j++){
                header->symbol_state[ind++] = S;
            }
            header->max = S;
        }
        for (uint i = S0; i <= (S+1) ; i++)
        {
            header->freq[i] += header->freq[i-1];
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

    struct bit_reader * breader = initialise_bit_reader(my_reader);
    if(header->data != NULL) myfree(header->data);
    header->data = mymalloc(sizeof(uint32_t) * content_size);
    for(uint n = 0; n < content_size; n++){
        header->data[n] = (uint32_t)read_bits(bits, breader);
    }
    free_bit_reader(breader);
    for(i=0; i<len; i++)
    {
        S = header->symbol_state[state % m];
        ls = header->freq[S+1] - header->freq[S];
        bs = header->freq[S];
        block->data[block->size++] = S;
        state = ls * (state / m) + (state % m) - bs;
        while(state < m && (content_size-read) > 0){
            read++;
            state = (state << bits) + header->data[content_size-read];
        }
    }
    if(signature.symbol == SYMBOL_MSB){
        msb_bytes = mymalloc(sizeof(uint32_t) * post_size);
        struct bit_reader * breader = initialise_bit_reader(my_reader);
        for(uint i=0; i<post_size;i++)
            msb_bytes[i] = (uint32_t)read_bits(msb_bits, breader);
        free_bit_reader(breader);
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
