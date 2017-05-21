#include "bANS.h"


void bANS_encode(FILE * input_file, FILE * output_file)
{
    uint32_t block[BLOCK_LEN];
    size_t size;
    write_meta_header(input_file, output_file);
    size = fread(block, sizeof(uint32_t), BLOCK_LEN, input_file);
    while(size > 0){
        process_encode_block(block, size, output_file);
        size = fread(block, sizeof(uint32_t), BLOCK_LEN, input_file);
    }
}

void write_meta_header(FILE * input_file, FILE * output_file)
{
    size_t size;
    uint32_t no_blocks;
    fseek(input_file, 0, SEEK_END);
    size = ftell(input_file);
    no_blocks = (size / sizeof(uint32_t)) / BLOCK_LEN;
    if(((size / sizeof(uint32_t)) % BLOCK_LEN) > 0)
        no_blocks += 1;
    fseek(input_file, 0, SEEK_SET);
    fwrite(&no_blocks, sizeof(uint32_t), 1, output_file);
}

void process_encode_block(uint32_t * block, size_t block_size, FILE * output_file){
    uint64_t state = block_size;
    struct block_header header;
    header = calculate_block_header(block, block_size);
    size_t i = block_size;
    struct output_obj output = get_output_obj(output_file);
    while(i > 0){
        i--;
        process_encode(block[i], &state, &header, &output);
    }
    write_block(state, &header, &output);
}

void process_encode(uint32_t symbol, uint64_t * state, struct block_header * header, struct output_obj * output)
{
    size_t index = header->index[symbol];
    uint64_t ls = header->freq[index];
    uint64_t bs = header->cumalative_freq[index];
    uint64_t Is = header->I_max[index];
    while(*state > Is){
        write_output(state, output);
    }
    *state = header->m * (*state / ls) + bs + (*state % ls);
    header->block_len += 1;
}


void write_output(uint64_t * state, struct output_obj * output)
{
    output->output[output->head] = *state % B;
    *state = *state / B;
    output->head += 1;
}

void write_block(uint64_t state, struct block_header * header, struct output_obj * output)
{
    fwrite(&state, sizeof(uint64_t), 1, output->file);
    fwrite(&header->no_symbols, sizeof(size_t), 1, output->file);
    fwrite(&header->m, sizeof(size_t), 1, output->file);
    fwrite(&header->block_len, sizeof(size_t), 1, output->file);
    fwrite(&output->head, sizeof(size_t), 1, output->file);
    fwrite(header->symbol, sizeof(uint32_t), header->no_symbols, output->file);
    fwrite(header->freq, sizeof(uint64_t), header->no_symbols, output->file);
    fwrite(output->output, sizeof(unsigned char), output->head, output->file);
}
struct output_obj get_output_obj(FILE * output_file)
{
    struct output_obj output;
    output.head = 0;
    output.file = output_file;
    output.output = malloc(sizeof(unsigned char) * (BLOCK_LEN << 2));
    return output;
}
struct block_header calculate_block_header(uint32_t * block, size_t block_size)
{
    struct block_header header;
    uint32_t * map = calloc(SYMBOL_RANGE, sizeof(uint32_t));
    size_t i = 0;
    size_t ind = 0;
    uint32_t max_symbol = 0;
    uint64_t cumalative_freq = 0;
    while(i<block_size){
        if (max_symbol < block[i])
            max_symbol = block[i];
        map[block[i]] += 1;
        i++;
    }
    i = 0;
    while(i <= max_symbol){
        if(map[i] > 0){
            ind++;
        }
        i++;
    }
    header.no_symbols = ind;
    ind=0;
    i=0;
    header.index = malloc(sizeof(size_t) * SYMBOL_RANGE);
    header.symbol = malloc(sizeof(uint32_t) * header.no_symbols);
    header.freq = malloc(sizeof(uint64_t) * header.no_symbols);
    header.cumalative_freq = malloc(sizeof(uint64_t) * header.no_symbols);
    header.I_max = malloc(sizeof(uint64_t) * header.no_symbols);
    while(i <= max_symbol){
        if(map[i] > 0){
            header.index[i] = ind;
            header.symbol[ind] = i;
            header.freq[ind] = map[i];
            header.cumalative_freq[ind] = cumalative_freq;
            header.I_max[ind] = (map[i] * B) - 1;
            cumalative_freq += map[i];
            ind++;
        }
        i++;
    }
    header.m = block_size;
    header.no_symbols = ind;
    header.block_len = 0;
    return header;
}











void bANS_decode(FILE * input_file, FILE * output_file)
{
    uint32_t no_blocks;
    uint32_t i = 0;
    fread(&no_blocks, sizeof(uint32_t), 1, input_file);
    while(i < no_blocks){
        process_decode_block(input_file, output_file);
        i++;
    }
}

void process_decode_block(FILE * input_file, FILE * output_file)
{
    size_t i = 0;
    uint64_t state;
    uint32_t * output;
    fread(&state, sizeof(uint64_t), 1, input_file);

    struct block_header header = read_block_header(input_file);
    output = malloc(sizeof(uint32_t) * header.block_len);
    struct output_obj input;
    input.output = malloc(sizeof(unsigned char) * header.content_length);
    fread(input.output, sizeof(unsigned char), header.content_length, input_file);
    input.head = header.content_length;
    while(i < header.block_len){
        process_decode(&state, &header, &output[i]);
        while(state < header.block_len){
            input.head -= 1;
            state = (state << Bbits) + input.output[input.head];
        }
        i++;
    }
    fwrite(output, sizeof(uint32_t), header.block_len, output_file);
}

void process_decode(uint64_t * state, struct block_header * header, uint32_t * output)
{
    size_t index = header->symbol_state[*state % header->block_len];
    uint64_t ls = header->freq[index];
    uint64_t bs = header->cumalative_freq[index];
    *output = header->symbol[index];
    *state = ls * (*state / header->m) + (*state % header->m) - bs;
}

struct block_header read_block_header(FILE * input_file)
{
    struct block_header header;
    size_t i = 0;
    size_t ind = 0;
    uint64_t cumalative_freq = 0;
    fread(&header.no_symbols, sizeof(size_t), 1, input_file);
    fread(&header.m, sizeof(size_t), 1, input_file);
    fread(&header.block_len, sizeof(size_t), 1, input_file);
    fread(&header.content_length, sizeof(size_t), 1, input_file);
    header.symbol = malloc(sizeof(uint32_t) * header.no_symbols);
    header.freq = malloc(sizeof(uint64_t) * header.no_symbols);
    header.cumalative_freq = malloc(sizeof(uint64_t) * header.no_symbols);
    header.symbol_state = malloc(sizeof(size_t) * BLOCK_LEN);
    fread(header.symbol, sizeof(uint32_t), header.no_symbols, input_file);
    fread(header.freq, sizeof(uint64_t), header.no_symbols, input_file);
    while(i < header.no_symbols){
        header.cumalative_freq[i] = cumalative_freq;
        cumalative_freq += header.freq[i];
        i++;
    }
    header.m = cumalative_freq;
    i = 0;
    while(i < header.block_len){
        if(ind < (header.no_symbols-1) && i >= header.cumalative_freq[ind + 1])
            ind += 1;
        header.symbol_state[i] = ind;
        i++;
    }
    return header;
}
