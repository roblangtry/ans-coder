#include "block.h"
uint block_No = 0;
uint32_t get_symbol(uint32_t input, coding_signature_t signature)
{
    if(signature.symbol == SYMBOL_DIRECT) return input;
    else if(signature.symbol == SYMBOL_MSB) return get_msb_symbol(input, signature.msb_bit_factor);
    else if(signature.symbol == SYMBOL_MSB_2) return get_msb_2_symbol(input, signature.msb_bit_factor);
    else exit(-1);
}
uint32_t get_usymbol(uint32_t input, coding_signature_t signature)
{
    if(signature.symbol == SYMBOL_DIRECT) return input;
    else if(signature.symbol == SYMBOL_MSB) return get_umsb_symbol(input, signature.msb_bit_factor);
    else if(signature.symbol == SYMBOL_MSB_2) return get_umsb_2_symbol(input, signature.msb_bit_factor);
    else exit(-1);
}

void generate_block_header(file_header_t * header, uint32_t size, coding_signature_t signature, struct prelude_code_data * metadata)
{
    uint32_t no_unique = 0;
    uint32_t symbol, n =0;
    uint32_t max = 0;
    header->translation = NULL;
    header->symbols = size;
    //read the block
    elias_encode(metadata, size);
    if(signature.translation == TRANSLATE_TRUE) build_translations_encoding(header, size, metadata);
    //clear the header
    if(signature.hashing == HASHING_STANDARD){
        myfree(header->freq);
        if(signature.symbol == SYMBOL_MSB) header->freq = mycalloc(get_msb_symbol(SYMBOL_MAP_SIZE, signature.msb_bit_factor)+1, sizeof(uint32_t));
        else if(signature.symbol == SYMBOL_MSB_2) header->freq = mycalloc(get_msb_2_symbol(SYMBOL_MAP_SIZE, signature.msb_bit_factor)+1, sizeof(uint32_t));
        else header->freq = mycalloc(header->global_max + BLOCK_SIZE + 1, sizeof(uint32_t));
    }
    else
    {
        if(signature.symbol == SYMBOL_MSB) header->freq_hash = sparse_hash_create(get_msb_symbol(SYMBOL_MAP_SIZE, signature.msb_bit_factor)+1);
        else if(signature.symbol == SYMBOL_MSB_2) header->freq_hash = sparse_hash_create(get_msb_2_symbol(SYMBOL_MAP_SIZE, signature.msb_bit_factor)+1);
        else header->freq_hash = sparse_hash_create(header->global_max + BLOCK_SIZE + 1);
    }
    header->max = 0;
    for(uint i=0; i<size; i++)
    {
        if(signature.translation == TRANSLATE_TRUE) symbol = get_symbol(header->translation[header->data[i]], signature);
        else symbol = get_symbol(header->data[i], signature);
        if(signature.hashing == HASHING_STANDARD){
            if(!header->freq[symbol+1]) no_unique++;
            header->freq[symbol+1]++;
        }
        else
        {
            if(sparse_hash_increment(symbol+1, 1, header->freq_hash) == 1) no_unique++;
        }
        if(symbol > header->max) header->max = symbol;
    }
    max = header->max;
    if(header->global_max < max) header->global_max = max;
    //calculate cumalative frequency

    for(uint i=1; (i<=max+1); i++)
    {
        if(signature.hashing == HASHING_STANDARD) header->freq[i] += header->freq[i-1];
        else sparse_hash_increment(i, sparse_hash_get(i-1, header->freq_hash), header->freq_hash);
    }
    if(signature.translation != TRANSLATE_TRUE) elias_encode(metadata, no_unique);
    symbol = 0;
    uint32_t F = 0;
    uint32_t j = 0;
    for(uint i=0; i<no_unique; i++)
    {
        if(signature.hashing == HASHING_STANDARD) F = header->freq[j+1] - header->freq[j];
        else F = sparse_hash_get(j+1, header->freq_hash) - sparse_hash_get(j, header->freq_hash);
        while(!F){
            j++;
            if(signature.hashing == HASHING_STANDARD) F = header->freq[j+1] - header->freq[j];
            else F = sparse_hash_get(j+1, header->freq_hash) - sparse_hash_get(j, header->freq_hash);
        }
        if(signature.translation != TRANSLATE_TRUE) elias_encode(metadata, j - symbol);
        symbol = j++;
        if(signature.translation != TRANSLATE_TRUE) elias_encode(metadata, F);
    }
}
void read_block_heading(file_header_t * header, uint32_t * len, coding_signature_t signature, struct prelude_code_data * metadata)
{
    uint32_t S, F, ind = 0, j, cumalative, p, i;
    if(header->freq != NULL) myfree(header->freq);
    header->symbols = elias_decode(metadata);
    *len = header->symbols;
    header->unique_symbols = elias_decode(metadata);
    header->freq = mycalloc(header->unique_symbols + 2 , sizeof(uint32_t));
    header->symbol = mymalloc((header->unique_symbols+1) * sizeof(uint32_t));
    S = 0;
    for(i=0; i<header->unique_symbols; i++){
        p = elias_decode(metadata);
        S = p + S;
        F = elias_decode(metadata);
        header->freq[i] = F;
        header->symbol[i] = S;
        header->max = S;
    }
    if(signature.translation == TRANSLATE_TRUE)
    {
        build_translations_decoding(header, signature);
        for(i=0; i<header->unique_symbols; i++){
            F = header->freq[i];
        }
    }
    cumalative = 0;
    for (i = 0; i <= header->unique_symbols ; i++)
    {
        F = header->freq[i];
        header->freq[i] = cumalative;
        for(j = cumalative; j < F + cumalative; j++){
            header->symbol_state[j] = i;
        }
        cumalative = F + cumalative;
    }
}

void process_block(FILE * input_file, struct writer * my_writer, file_header_t * header, coding_signature_t signature)
{
    uint32_t size;
    uint32_t symbol;
    uint64_t state, ls, bs, Is, m, bits = signature.bit_factor, msb_bits = signature.msb_bit_factor;
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    int_page_t * ans_pages = get_int_page();
    int_page_t * msb_pages;
    bit_page_t * msb_2_pages;
    if(signature.symbol == SYMBOL_MSB) msb_pages = get_int_page();
    if(signature.symbol == SYMBOL_MSB_2) msb_2_pages = get_bit_page();
    size = fread(header->data, sizeof(uint32_t), BLOCK_SIZE, input_file);

    // ------------- //
    // PRE
    // ------------- //

    if(signature.header == HEADER_BLOCK)
    {
        generate_block_header(header, size, signature, metadata);
    }
    else if(signature.header == HEADER_SINGLE)
    {
        elias_encode(metadata, size);
    }

    state = header->symbols;
    m = header->symbols;
    for(int i=size-1; i>=0; i--)
    {
        if(i < 115 && i > 105){
            // printf("%u} S %u\n", i, header->data[i]);
            // sleep(1);
        }
        if(signature.translation == TRANSLATE_TRUE) symbol = get_symbol(header->translation[header->data[i]], signature);
        else symbol = get_symbol(header->data[i], signature);
        if(signature.hashing == HASHING_STANDARD){
            ls = header->freq[symbol+1] - header->freq[symbol];
            bs = header->freq[symbol];
        }
        else
        {
            ls = sparse_hash_get(symbol+1, header->freq_hash) - sparse_hash_get(symbol, header->freq_hash);
            bs = sparse_hash_get(symbol, header->freq_hash);
        }
        Is = (ls << bits) - 1;
        while(state > Is){
            add_to_int_page(state % (1 << bits), ans_pages);
            state = state >> bits;
        }
        state = m * (state / ls) + bs + (state % ls);
        if(signature.symbol == SYMBOL_MSB)
        {
            if(signature.translation == TRANSLATE_TRUE) stream_msb(header->translation[header->data[i]], msb_bits, msb_pages);
            else stream_msb(header->data[i], msb_bits, msb_pages);
        }
        if(signature.symbol == SYMBOL_MSB_2)
        {
            if(signature.translation == TRANSLATE_TRUE) stream_msb_2(header->translation[header->data[i]], msb_bits, msb_2_pages);
            else stream_msb_2(header->data[i], msb_bits, msb_2_pages);
        }
    }
    elias_encode(metadata, ans_pages->current_size);
    if(signature.symbol == SYMBOL_MSB){
        elias_encode(metadata, msb_pages->current_size);
    }
    if(signature.symbol == SYMBOL_MSB_2){
        elias_encode(metadata, msb_2_pages->current_size);
    }
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
    if(signature.symbol == SYMBOL_MSB_2){
        output_bit_page(my_writer, msb_2_pages);
        free_bit_page(msb_2_pages);
    }
    if(signature.translation == TRANSLATE_TRUE) myfree(header->translation);
    if(signature.hashing == HASHING_SPARSE) sparse_hash_free(header->freq_hash);
}

void read_block(struct reader * my_reader, file_header_t * header, coding_signature_t signature, data_block_t * block)
{
    uint64_t state, ls, bs, m, bits = signature.bit_factor, msb_bits = signature.msb_bit_factor;
    uint32_t S, content_size, post_size = 0;
    uint32_t read=0, len = 0;
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    uint32_t * msb_bytes = NULL;
    uint32_t byte;
    uint i, j, k;
    // printf("stage 1\n");
    if(signature.header == HEADER_BLOCK)
    {
        read_block_heading(header, &len, signature, metadata);
    }
    else if(signature.header == HEADER_SINGLE)
    {
        len = elias_decode(metadata);
    }
    // printf("stage 2\n");
    block->size = 0;
    m = header->symbols;
    content_size = elias_decode(metadata);
    if(signature.symbol == SYMBOL_MSB || signature.symbol == SYMBOL_MSB_2){
        post_size = elias_decode(metadata);
    }
    read_uint64_t(&state, my_reader);

    // printf("stage 3\n");
    struct bit_reader * breader = initialise_bit_reader(my_reader);
    if(header->data != NULL) myfree(header->data);
    header->data = mymalloc(sizeof(uint32_t) * content_size);
    // printf("stage 4\n");
    for(uint n = 0; n < content_size; n++){
        header->data[n] = (uint32_t)read_bits(bits, breader);
    }
    // printf("stage 5\n");
    free_bit_reader(breader);
    for(i=0; i<len; i++)
    {
        S = header->symbol_state[state % m];
        ls = header->freq[S+1] - header->freq[S];
        bs = header->freq[S];
        block->data[block->size++] = header->symbol[S];
        state = ls * (state / m) + (state % m) - bs;
        while(state < m && (content_size-read) > 0){
            read++;
            state = (state << bits) + header->data[content_size-read];
        }
    }
    printf("stage 6\n");
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
            j = (block->data[i] - 1) / (1<<msb_bits);
            // printf("j\n");
            block->data[i] -= (1<<msb_bits) * j;
            block->data[i] = block->data[i]<<(msb_bits*j);
            for(k = 0;k<j;k++){
                byte = msb_bytes[post_size--];
                if(!k) block->data[i] = block->data[i] + byte;
                else block->data[i] = block->data[i] + (byte << (msb_bits * k));
            }
            if(signature.translation == TRANSLATE_TRUE)block->data[i] = header->translation[block->data[i]-1];
            i++;
        }
    }
    else if(signature.symbol == SYMBOL_MSB_2){
        struct bit_reader * breader = initialise_bit_reader(my_reader);
        if(len)
        {
        i = len - 1;
        post_size--;
        while(i >= 0)
        {
    // printf("stage 7a (%u - 1) / (1 << %u)\n", block->data[i], msb_bits);
            j = (block->data[i] - 1) / (1<<msb_bits);
            // printf("%u] (%u,%u)\n", i, block->data[i], j);
            // printf("IF(%u != 0 && %u != 0) = (%u, %u)\n", j,block->data[i],j != 0,block->data[i] != 0);
            if(j != 0 && block->data[i] != 0){
                block->data[i] -= (1<<msb_bits) * j;
                block->data[i] = block->data[i]<<j;
                byte = (uint32_t)read_bits(j, breader);
                block->data[i] = block->data[i] + byte;
            }
            // if(i < 115 && i > 105) sleep(1);

            // if(i < 115 && i > 105) printf("%u} [%u] %u - %u\n", i, j, byte, block->data[i]);

            if(signature.translation == TRANSLATE_TRUE)block->data[i] = header->translation[block->data[i]-1];
    // printf("stage 7j\n");
            if(i == 0) break;
    // printf("stage 7k\n");
            i--;
    // printf("stage 7l\n");
        }
        }
        free_bit_reader(breader);
        myfree(msb_bytes);
    }
    // printf("stage 7\n");
    if(signature.header == HEADER_BLOCK) myfree(header->symbol);
    free_metadata(metadata);
    if(signature.translation == TRANSLATE_TRUE) myfree(header->translation);
}

void output_to_file(FILE * output_file, data_block_t * data)
{
    fwrite(data->data, sizeof(uint32_t), data->size, output_file);
}
