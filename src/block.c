#include "block.h"

void generate_block_header(file_header_t * header, uint32_t size, coding_signature_t signature, struct prelude_code_data * metadata)
{
    // sleep(1);
    uint32_t no_unique = 0;
    uint32_t symbol;
    uint32_t max = 0;
    uint32_t *this, *that, *top;
    header->translation = NULL;
    header->symbols = size;
    //read the block
    elias_encode(metadata, size);
    // printf("E s %u\n", size);
    //clear the header
    FREE(header->freq);
    if(signature.symbol == SYMBOL_MSB) header->freq = mycalloc(get_msb_symbol(SYMBOL_MAP_SIZE, signature.msb_bit_factor)+1024, sizeof(uint32_t));
    else if(signature.symbol == SYMBOL_MSB_2) header->freq = mycalloc(get_msb_2_symbol(SYMBOL_MAP_SIZE, signature.msb_bit_factor)+1024, sizeof(uint32_t));
    else header->freq = mycalloc(header->global_max + BLOCK_SIZE + 1, sizeof(uint32_t));
    if(translating(signature.translation)) build_translations_encoding(header, size, metadata, signature);
    else{
        this = header->data;
        top = header->data + size;
        while(this < top)
        {
            if(translating(signature.translation)) symbol = get_symbol(header->translation[(*this++)], signature);
            else symbol = get_symbol((*this++), signature);
            if(!header->freq[symbol+1]) no_unique++;
            header->freq[symbol+1]++;
            if(symbol > header->max) header->max = symbol;
        }
    }
    if(translating(signature.translation)) no_unique = header->nu;
    max = header->max;
    if(header->global_max < max) header->global_max = max;
    //calculate cumalative frequency
    if(!translating(signature.translation))
    {
        elias_encode(metadata, no_unique);
        symbol = 0;
        uint32_t *x = header->freq,*y = header->freq+1;
        for(uint i=0; i<no_unique; i++)
        {
            while(!*(++x));
            elias_encode(metadata, (x-y) - symbol);
            symbol = (x-y);
            elias_encode(metadata, *x);
        }
    }
    this = header->freq + 1;
    that = header->freq;
    top = header->freq+max+2;
    while(this<top)
        (*this++) += (*that++);
}
void read_block_heading(file_header_t * header, uint32_t * len, coding_signature_t signature, struct prelude_code_data * metadata)
{
    // sleep(1);
    uint32_t S, F, j, cumalative, p, i, *f,*fT,*s,*sT;
    if(header->freq != NULL) FREE(header->freq);
    header->symbols = elias_decode(metadata);
    // printf("D s %u\n", header->symbols);
    *len = header->symbols;
    header->unique_symbols = elias_decode(metadata);
    // printf("D p %u\n", header->unique_symbols);
    header->freq = mycalloc(header->unique_symbols + 2 , sizeof(uint32_t));
    header->symbol = mymalloc((header->unique_symbols+1) * sizeof(uint32_t));
    S = 0;
    f = header->freq;
    fT = f+header->unique_symbols;
    s = header->symbol;
    while(f<fT){
        p = elias_decode(metadata);
        F = elias_decode(metadata);
        S = p + S;
        *f++ = F;
        *s++ = S;
        header->max = S;
    }
    if(translating(signature.translation))
    {
        build_translations_decoding(header, signature, metadata);
    }
    cumalative = 0;
    f = header->freq;
    fT= f+header->unique_symbols;
    s = header->symbol_state;
    sT = s;
    i=0;
    while(f<=fT)
    {
        F = *f;
        // printf("%u] %u\n", i, F);
        *f++ = cumalative;
        sT = sT + F;
        while(s<sT){
            *s++ = i;
        }
        // printf("%u] %u (%u - %u)\n", i, F, cumalative, cumalative + F);
        // sleep(1);
        i++;
        cumalative = F + cumalative;
    }
}

void process_block(FILE * input_file, struct writer * my_writer, file_header_t * header, coding_signature_t signature)
{
    uint32_t size;
    uint32_t symbol, *this, *bot;
    uint64_t state, ls, bs, Is, m, bits = signature.bit_factor, msb_bits = signature.msb_bit_factor;
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    bint_page_t * ans_pages = get_bint_page();
    bint_page_t * msb_pages;
    bit_page_t * msb_2_pages;
    if(signature.symbol == SYMBOL_MSB) msb_pages = get_bint_page();
    if(signature.symbol == SYMBOL_MSB_2) msb_2_pages = get_bit_page();
    size = fread(header->data, sizeof(uint32_t), BLOCK_SIZE, input_file);

    // ------------- //
    // PRE
    // ------------- //

    if(signature.header == HEADER_BLOCK)
        generate_block_header(header, size, signature, metadata);
    else if(signature.header == HEADER_SINGLE)
        elias_encode(metadata, size);

    state = header->symbols;
    m = header->symbols;
    this = header->data+size-1;
    bot = header->data;
    while(this>=bot)
    {
        if(translating(signature.translation)) symbol = get_symbol(header->translation[*this], signature);
        else symbol = get_symbol(*this, signature);
        ls = header->freq[symbol+1] - header->freq[symbol];
        bs = header->freq[symbol];
        Is = (ls << bits) - 1;
        while(state > Is){
            add_to_bint_page(state % (1 << bits), bits, ans_pages);
            state = state >> bits;
        }
        state = m * (state / ls) + bs + (state % ls);
        if(signature.symbol == SYMBOL_MSB)
        {
            if(translating(signature.translation)) stream_msb(header->translation[*this], msb_bits, msb_pages);
            else stream_msb(*this, msb_bits, msb_pages);
        }
        if(signature.symbol == SYMBOL_MSB_2)
        {
            if(translating(signature.translation)) stream_msb_2(header->translation[*this], msb_bits, msb_2_pages);
            else stream_msb_2(*this, msb_bits, msb_2_pages);
        }
        this--;
    }
    elias_encode(metadata, ans_pages->no_writes);
    elias_flush(metadata);
    free_metadata(metadata);
    write_uint64_t(state, my_writer);

    // ------------- //
    // Content
    // ------------- //
    output_bint_page(my_writer, ans_pages, bits);
    ans_size(ans_pages->current_size * 4);
    // ------------- //
    // Post
    // ------------- //
    free_bint_page(ans_pages);
    if(signature.symbol == SYMBOL_MSB){
        msb_size(msb_pages->current_size * 4);
        output_bint_page(my_writer, msb_pages, msb_bits);
        free_bint_page(msb_pages);
    }
    if(signature.symbol == SYMBOL_MSB_2){
        output_bit_page(my_writer, msb_2_pages);
        free_bit_page(msb_2_pages);
    }
    if(translating(signature.translation)) FREE(header->translation);
    if(signature.hashing == HASHING_SPARSE) sparse_hash_free(header->freq_hash);
}

void read_block(struct reader * my_reader, file_header_t * header, coding_signature_t signature, data_block_t * block)
{
    uint64_t state, ls, bs, m, bits = signature.bit_factor, msb_bits = signature.msb_bit_factor;
    uint32_t S, content_size, *head, *top, *W, *T;
    uint32_t len = 0;
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    uint32_t byte;
    uint i, j, k;
    if(signature.header == HEADER_BLOCK)
    {
        read_block_heading(header, &len, signature, metadata);
    }
    else if(signature.header == HEADER_SINGLE)
    {
        len = elias_decode(metadata);
    }
    m = header->symbols;
    content_size = elias_decode(metadata);
    read_uint64_t(&state, my_reader);
    struct bit_reader * breader = initialise_bit_reader(my_reader);
    if(header->data != NULL) FREE(header->data);
    header->data = mymalloc(sizeof(uint32_t) * content_size);
    head = header->data;
    top = header->data+content_size;
    while(head<top)
        *head++ = (uint32_t)read_bits(bits, breader);
    head--;
    W = block->data;
    T = block->data + len;
    free_bit_reader(breader);
    while(W < T)
    {
        S = header->symbol_state[state % m];
        ls = header->freq[S+1] - header->freq[S];
        bs = header->freq[S];
        *W++ = header->symbol[S];
        state = ls * (state / m) + (state % m) - bs;
        while(state < m)
            state = (state << bits) + *head--;
    }
    block->size = len;
    if(signature.symbol == SYMBOL_MSB || signature.symbol == SYMBOL_MSB_2){
        struct bit_reader * breader = initialise_bit_reader(my_reader);
        if(len)
        {
            W--;
            T = block->data;
            while(W >= T)
            {
                j = (*W - 1)>>msb_bits;

                if(signature.symbol == SYMBOL_MSB)
                {
                    *W -= j<<msb_bits;
                    *W = *W<<(msb_bits*j);
                    if(j > 0){
                        for(k = j-1;k>=0;k--){
                            byte = (uint32_t)read_bits(msb_bits, breader);
                            if(!k) *W = *W + byte;
                            else *W = *W + (byte << (msb_bits * k));
                            if(!k) break;
                        }
                    }
                }
                else
                {
                    if(j != 0 && *W != 0){
                        *W -= j<<msb_bits;
                        *W = *W<<j;
                        byte = (uint32_t)read_bits(j, breader);
                        *W = *W + byte;
                    }
                }
                if(translating(signature.translation))*W = header->translation[*W-1];
                W--;
            }
        }
        free_bit_reader(breader);
    }
    if(signature.header == HEADER_BLOCK) FREE(header->symbol);
    free_metadata(metadata);
    if(translating(signature.translation)) FREE(header->translation);
}

void output_to_file(FILE * output_file, data_block_t * data)
{
    fwrite(data->data, sizeof(uint32_t), data->size, output_file);
}
