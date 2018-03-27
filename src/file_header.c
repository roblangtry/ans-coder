#include "file_header.h"

void preprocess_file(FILE * input_file, coding_signature_t signature, file_header_t * header)
{
    uint32_t no_blocks = 0;
    uint64_t cumal = 0;
    uint32_t msb_bits = signature.msb_bit_factor, symbol;
    size_t prev = 1;
    header->max = 0;
    header->symbols = 0;
    header->unique_symbols = 0;
    uint32_t * block;
    if(signature.header == HEADER_SINGLE){
        block = mymalloc(sizeof(uint32_t) * BLOCK_SIZE);
        while(prev > 0)
        {
            prev = fread (block, sizeof(uint32_t), BLOCK_SIZE, input_file);
            for(uint i=0; i<prev; i++)
            {
                if(signature.symbol == SYMBOL_DIRECT) symbol = block[i];
                else if(signature.symbol == SYMBOL_MSB) symbol = get_msb_symbol(block[i], msb_bits);
                else if(signature.symbol == SYMBOL_MSB_2) symbol = get_msb_2_symbol(block[i], msb_bits);
                else exit(-1);
                header->freq[symbol]++;
                if(symbol>header->max) header->max=symbol;
                header->symbols++;
            }
            if(prev > 0) no_blocks++;
        }
        header->unique_symbols = header->max+1;
        for(uint i=0; i<=header->max; i++)
        {
            header->cumalative_freq[i] = cumal;
            cumal = cumal + header->freq[i];
        }
        FREE(block);
        block = NULL;
    } else if (signature.header == HEADER_BLOCK){
        fseek(input_file, 0, SEEK_END);
        no_blocks = ftell(input_file)/BLOCK_SIZE;
        if(ftell(input_file)%BLOCK_SIZE) no_blocks++;
    }
    header->no_blocks = no_blocks;
    fseek(input_file, 0, SEEK_SET);
}

void output_file_header(struct writer * my_writer, file_header_t * header, coding_signature_t signature)
{
    write_uint32_t(MAGIC, my_writer);
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    elias_encode(metadata, signature.symbol);
    if(signature.symbol == SYMBOL_MSB || signature.symbol == SYMBOL_MSB_2)
        elias_encode(metadata, signature.msb_bit_factor);
    elias_encode(metadata, signature.header);
    elias_encode(metadata, signature.ans);
    elias_encode(metadata, signature.bit_factor);
    elias_encode(metadata, signature.translation);
    elias_encode(metadata, header->no_blocks);
    if(signature.header == HEADER_SINGLE){
        elias_encode(metadata, header->max);
        for(uint i = 0; i<=header->max; i++)
        {
            elias_encode(metadata, header->freq[i]);
        }
    }
    elias_flush(metadata);
    free_metadata(metadata);
}
void read_signature(struct reader * my_reader, coding_signature_t * signature, struct prelude_code_data * metadata)
{
    uint32_t magic = 0;
    read_uint32_t(&magic, my_reader);
    if(magic != MAGIC){
        fprintf(stderr, "This file doesn't appear to have been encoded properly (%d)\n", magic);
        exit(-1);
    }
    (*signature).symbol = elias_decode(metadata);
    if((*signature).symbol == SYMBOL_MSB || (*signature).symbol == SYMBOL_MSB_2)
        (*signature).msb_bit_factor = elias_decode(metadata);
    (*signature).header = elias_decode(metadata);
    (*signature).ans = elias_decode(metadata);
    (*signature).bit_factor = elias_decode(metadata);
    (*signature).translation = elias_decode(metadata);
}
void read_file_header(struct reader * my_reader, coding_signature_t signature, file_header_t * header, struct prelude_code_data * metadata)
{
    uint64_t total = 0, k = 0, cumal = 0;
    header->no_blocks = elias_decode(metadata);
    if(signature.header == HEADER_SINGLE){
        header->max = elias_decode(metadata);
        header->symbols = 0;
        header->unique_symbols = header->max + 1;
        for(uint i = 0; i<=header->max; i++)
        {
            header->freq[i] = elias_decode(metadata);

            total += header->freq[i];
        }
        header->symbols = total;
        header->symbol_state = mymalloc(sizeof(uint32_t) * total);
        for(uint i = 0; i<=header->max; i++)
        {
            for(uint j = 0; j < header->freq[i]; j++)
            {
                header->symbol_state[k++] = i;
            }
            header->cumalative_freq[i] = cumal;
            cumal = cumal + header->freq[i];
        }
    }
    free_metadata(metadata);
}