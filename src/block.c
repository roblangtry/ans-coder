#include "block.h"

void generate_block_header(file_header_t * header, uint32_t size, coding_signature_t signature, struct prelude_code_data * metadata)
{
    uint32_t no_unique = 0;
    uint32_t symbol=0;
    uint32_t max = 0;
    uint32_t *this=NULL, *that=NULL, *top=NULL;
    header->translation = NULL;
    header->symbols = size;
    //read the block
    elias_encode(metadata, size);
    //clear the header
    FREE(header->freq);
    if(signature.symbol == SYMBOL_MSB) header->freq = mycalloc(get_msb_symbol(SYMBOL_MAP_SIZE, signature.msb_bit_factor)+1024, sizeof(uint32_t));
    else if(signature.symbol == SYMBOL_MSB_2) header->freq = mycalloc(get_msb_2_symbol(SYMBOL_MAP_SIZE, signature.msb_bit_factor)+1024, sizeof(uint32_t));
    else header->freq = mycalloc(header->global_max + BLOCK_SIZE + 1, sizeof(uint32_t));
    if(translating(signature.translation)) build_translations_encoding(header, size, metadata, signature);
    else{
        this = header->data;
        top = header->data + size;
        if(signature.symbol == SYMBOL_MSB)
            while(this < top)
            {
                if(translating(signature.translation)) symbol = get_msb_symbol(header->translation[(*this++)], signature.msb_bit_factor);
                else symbol = get_msb_symbol((*this++), signature.msb_bit_factor);
                if(!header->freq[symbol+1]) no_unique++;
                header->freq[symbol+1]++;
                if(symbol > header->max) header->max = symbol;
            }
        else if(signature.symbol == SYMBOL_MSB_2)
            while(this < top)
            {
                if(translating(signature.translation)) symbol = get_msb_2_symbol(header->translation[(*this++)], signature.msb_bit_factor);
                else symbol = get_msb_2_symbol((*this++), signature.msb_bit_factor);
                if(!header->freq[symbol+1]) no_unique++;
                header->freq[symbol+1]++;
                if(symbol > header->max) header->max = symbol;
            }
        else
            while(this < top)
            {
                if(translating(signature.translation)) symbol = (*this++);
                else symbol = (*this++);
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
        while(no_unique--)
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
    uint32_t S = 0, F = 0, cumalative = 0, p = 0, i = 0, *f = NULL,*fT = NULL,*s = NULL,*sT = NULL, *ssT = NULL;
    if(header->freq != NULL) FREE(header->freq);
    header->symbols = elias_decode(metadata);
    *len = header->symbols;
    header->unique_symbols = elias_decode(metadata);
    header->freq = mycalloc(header->unique_symbols + 2 , sizeof(uint32_t));
    header->symbol = mymalloc((header->unique_symbols+2) * sizeof(uint32_t));
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
    ssT = header->symbol_state+BLOCK_SIZE;
    i=0;
    while(f<=fT)
    {
        F = *f;
        *f++ = cumalative;
        sT = sT + F;
        while(s<sT && s<ssT){
            *s++ = i;
        }
        i++;
        cumalative = F + cumalative;
    }
}

uint64_t direct_ans_component(file_header_t * header, bint_page_t * ans_pages,
    bint_page_t * msb_pages, bit_page_t * msb_2_pages, uint32_t bits, uint32_t msb_bits, uint32_t size, coding_signature_t signature)
{
    uint32_t *data = header->data;
    uint32_t *freq = header->freq;
    uint64_t state = header->symbols;
    uint64_t m = header->symbols;
    uint32_t * this = data+size-1;
    uint32_t * bot = data;
    uint64_t ls, bs, Is;
    uint32_t symbol;
    while(this>=bot)
    {
        symbol = *this;
        bs = freq[symbol];
        ls = freq[symbol+1] - bs;
        Is = (ls << bits) - 1;
        while(state > Is){
            add_to_bint_page(state % (1 << bits), bits, ans_pages);
            state = state >> bits;
        }
        state = m * (state / ls) + bs + (state % ls);
        this--;
    }
    return state;
}

uint64_t msb_ans_component(file_header_t * header, bint_page_t * ans_pages,
    bint_page_t * msb_pages, bit_page_t * msb_2_pages, uint32_t bits, uint32_t msb_bits, uint32_t size, coding_signature_t signature)
{
    uint32_t *data = header->data;
    uint32_t *freq = header->freq;
    uint64_t state = header->symbols;
    uint64_t m = header->symbols;
    uint32_t * this = data+size-1;
    uint32_t * bot = data;
    uint64_t ls, bs, Is;
    uint32_t symbol;
    while(this>=bot)
    {
        symbol = get_msb_symbol(*this, msb_bits);
        bs = freq[symbol];
        ls = freq[symbol+1] - bs;
        Is = (ls << bits) - 1;
        while(state > Is){
            add_to_bint_page(state % (1 << bits), bits, ans_pages);
            state = state >> bits;
        }
        state = m * (state / ls) + bs + (state % ls);
        stream_msb(*this, msb_bits, msb_pages);
        this--;
    }
    return state;
}

uint64_t msb2_ans_component(file_header_t * header, bint_page_t * ans_pages,
    bint_page_t * msb_pages, bit_page_t * msb_2_pages, uint32_t bits, uint32_t msb_bits, uint32_t size, coding_signature_t signature)
{
    uint32_t *data = header->data;
    uint32_t *freq = header->freq;
    uint64_t state = header->symbols;
    uint64_t m = header->symbols;
    uint32_t * this = data+size-1;
    uint32_t * bot = data;
    uint64_t ls, bs, Is;
    uint32_t symbol;
    while(this>=bot)
    {
        symbol = get_msb_2_symbol(*this, msb_bits);
        bs = freq[symbol];
        ls = freq[symbol+1] - bs;
        Is = (ls << bits) - 1;
        while(state > Is){
            add_to_bint_page(state % (1 << bits), bits, ans_pages);
            state = state >> bits;
        }
        state = m * (state / ls) + bs + (state % ls);
        stream_msb_2(*this, msb_bits, msb_2_pages);
        this--;
    }
    return state;
}











uint64_t direct_translate_ans_component(file_header_t * header, bint_page_t * ans_pages,
    bint_page_t * msb_pages, bit_page_t * msb_2_pages, uint32_t bits, uint32_t msb_bits, uint32_t size, coding_signature_t signature)
{
    uint32_t *data = header->data;
    uint32_t *translate = header->translation;
    uint32_t *freq = header->freq;
    uint64_t state = header->symbols;
    uint64_t m = header->symbols;
    uint32_t * this = data+size-1;
    uint32_t * bot = data;
    uint64_t ls, bs, Is;
    uint32_t symbol;
    while(this>=bot)
    {
        symbol = translate[*this];
        bs = freq[symbol];
        ls = freq[symbol+1] - bs;
        Is = (ls << bits) - 1;
        while(state > Is){
            add_to_bint_page(state % (1 << bits), bits, ans_pages);
            state = state >> bits;
        }
        state = m * (state / ls) + bs + (state % ls);
        this--;
    }
    return state;
}

uint64_t msb_translate_ans_component(file_header_t * header, bint_page_t * ans_pages,
    bint_page_t * msb_pages, bit_page_t * msb_2_pages, uint32_t bits, uint32_t msb_bits, uint32_t size, coding_signature_t signature)
{
    uint32_t *data = header->data;
    uint32_t *translate = header->translation;
    uint32_t *freq = header->freq;
    uint64_t state = header->symbols;
    uint64_t m = header->symbols;
    uint32_t * this = data+size-1;
    uint32_t * bot = data;
    uint64_t ls, bs, Is;
    uint32_t symbol;
    while(this>=bot)
    {
        symbol = get_msb_symbol(translate[*this], msb_bits);
        bs = freq[symbol];
        ls = freq[symbol+1] - bs;
        Is = (ls << bits) - 1;
        while(state > Is){
            add_to_bint_page(state % (1 << bits), bits, ans_pages);
            state = state >> bits;
        }
        state = m * (state / ls) + bs + (state % ls);
        stream_msb(translate[*this], msb_bits, msb_pages);
        this--;
    }
    return state;
}

uint64_t msb2_translate_ans_component(file_header_t * header, bint_page_t * ans_pages,
    bint_page_t * msb_pages, bit_page_t * msb_2_pages, uint32_t bits, uint32_t msb_bits, uint32_t size, coding_signature_t signature)
{
    uint32_t *data = header->data;
    uint32_t *translate = header->translation;
    uint32_t *freq = header->freq;
    uint64_t state = header->symbols;
    uint64_t m = header->symbols;
    uint32_t * this = data+size-1;
    uint32_t * bot = data;
    uint64_t ls, bs, Is;
    uint32_t symbol;
    while(this>=bot)
    {
        symbol = get_msb_2_symbol(translate[*this], msb_bits);
        bs = freq[symbol];
        ls = freq[symbol+1] - bs;
        Is = (ls << bits) - 1;
        while(state > Is){
            add_to_bint_page(state % (1 << bits), bits, ans_pages);
            state = state >> bits;
        }
        state = m * (state / ls) + bs + (state % ls);
        stream_msb_2(translate[*this], msb_bits, msb_2_pages);
        this--;
    }
    return state;
}











uint64_t ans_component(file_header_t * header, bint_page_t * ans_pages,
    bint_page_t * msb_pages, bit_page_t * msb_2_pages, uint32_t bits, uint32_t msb_bits, uint32_t size, coding_signature_t signature)
{
    if(translating(signature.translation)){
        if(signature.symbol == SYMBOL_MSB)
            return msb_translate_ans_component(header, ans_pages, msb_pages, msb_2_pages, bits, msb_bits, size, signature);
        else if(signature.symbol == SYMBOL_MSB_2)
            return msb2_translate_ans_component(header, ans_pages, msb_pages, msb_2_pages, bits, msb_bits, size, signature);
        else
            return direct_translate_ans_component(header, ans_pages, msb_pages, msb_2_pages, bits, msb_bits, size, signature);
    }
    else{
        if(signature.symbol == SYMBOL_MSB)
            return msb_ans_component(header, ans_pages, msb_pages, msb_2_pages, bits, msb_bits, size, signature);
        else if(signature.symbol == SYMBOL_MSB_2)
            return msb2_ans_component(header, ans_pages, msb_pages, msb_2_pages, bits, msb_bits, size, signature);
        else
            return direct_ans_component(header, ans_pages, msb_pages, msb_2_pages, bits, msb_bits, size, signature);
    }
}

void process_block(FILE * input_file, struct writer * my_writer, file_header_t * header, coding_signature_t signature)
{
    uint32_t size = 0;
    uint64_t state = 0, bits = signature.bit_factor, msb_bits = signature.msb_bit_factor;
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    bint_page_t * ans_pages = get_bint_page();
    bint_page_t * msb_pages = NULL;
    bit_page_t * msb_2_pages = NULL;
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

    state = ans_component(header, ans_pages, msb_pages, msb_2_pages, bits, msb_bits, size, signature);
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
    uint64_t state = 0, ls = 0, bs = 0, m = 0, bits = signature.bit_factor, msb_bits = signature.msb_bit_factor;
    uint32_t S = 0, content_size = 0, *head = NULL, *top = NULL, *W = NULL, *T = NULL;
    uint32_t len = 0;
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    uint j = 0;
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
        while(state < m){
            state = (state << bits) + *head--;
        }
    }
    block->size = len;
    if(signature.symbol == SYMBOL_MSB || signature.symbol == SYMBOL_MSB_2){
        struct bit_reader * breader = initialise_bit_reader(my_reader);
        W--;
        T = block->data;
        while(W >= T)
        {
            j = (*W - 1)>>msb_bits;
            if(j){
                *W -= j<<msb_bits;
                if(signature.symbol == SYMBOL_MSB) j = j * msb_bits;
                *W = *W<<j;
                *W += (uint32_t)read_bits(j, breader);
            }
            if(signature.translation == TRANSLATE_PERMUTATION_PARTIAL)
            {
                if(*W <= header->translate_k){
                    *W = header->translation[*W-1];
                }
                else{
                    *W = (*W) - header->translate_k;
                }
            }
            else if(translating(signature.translation))*W = header->translation[*W-1];
            W--;
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
