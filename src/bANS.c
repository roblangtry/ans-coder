#include "bANS.h"


void bANS_encode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions, int flag)
{
    uint32_t block[BLOCK_SIZE];
    size_t size;
    struct writer * my_writer = initialise_writer(output_file);
    write_meta_header(input_file, my_writer);
    size = fread(block, sizeof(uint32_t), BLOCK_SIZE, input_file);
    while(size > 0){
        process_encode_block(block, size, my_writer, my_prelude_functions);
        size = fread(block, sizeof(uint32_t), BLOCK_SIZE, input_file);
    }
    flush_writer(my_writer);
}

void write_meta_header(FILE * input_file, struct writer * my_writer)
{
    size_t size;
    uint32_t no_blocks;
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    fseek(input_file, 0, SEEK_END);
    size = ftell(input_file);
    no_blocks = (size / sizeof(uint32_t)) / BLOCK_SIZE;
    if(((size / sizeof(uint32_t)) % BLOCK_SIZE) > 0)
        no_blocks += 1;
    fseek(input_file, 0, SEEK_SET);
    vbyte_encode(metadata, no_blocks);
}

void process_encode_block(uint32_t * block, size_t block_size, struct writer * my_writer, struct prelude_functions * my_prelude_functions){
    uint64_t state = block_size;
    struct block_header header;
    header = calculate_block_header(block, block_size);
    size_t i = block_size;
    struct output_obj output = get_output_obj(NULL);
    while(i > 0){
        i--;
        process_encode(block[i], &state, &header, &output);
    }
    write_block(state, &header, &output, my_writer, my_prelude_functions);
    clear_block_header(header);
    free_output_obj(output);
}

void process_encode(uint32_t symbol, uint64_t * state, struct block_header * header, struct output_obj * output)
{
    size_t index = get_symbol_index(symbol, header->index);
    uint32_t ls = header->freq[index];
    uint32_t bs = header->cumalative_freq[index];
    uint32_t Is = header->I_max[index];
    while(*state > Is){
        write_output(state, output);
    }
    *state = header->m * (*state / ls) + bs + (*state % ls);
    header->block_len += 1;
}


void write_output(uint64_t * state, struct output_obj * output)
{
    output->output[output->head] = *state % B;
    *state = *state >> Bbits;
    output->head += 1;
}

void write_block(uint64_t state, struct block_header * header, struct output_obj * output, struct writer * my_writer, struct prelude_functions * my_prelude_functions)
{
    write_symbol_prelude(header->symbol, header->freq, &header->no_symbols, &state, &output->head, my_writer, my_prelude_functions);
    write_bytes(output->output, output->head, my_writer);
}
void free_output_obj(struct output_obj obj)
{
    free(obj.output);
}
struct output_obj get_output_obj(FILE * output_file)
{
    struct output_obj output;
    output.head = 0;
    output.file = output_file;
    output.output = malloc(sizeof(unsigned char) * (BLOCK_SIZE << 2));
    return output;
}
lookup_t * build_lookup()
{
    int i;
    lookup_t * index = (lookup_t *)malloc(sizeof(lookup_t) * SYMBOL_MAP_SIZE);
    for(i=0;i<SYMBOL_MAP_SIZE;i++)
    {
        index[i].index = -1;
    }
    return index;
}
int get_symbol_index(int value, lookup_t * index)
{
    int i = value % SYMBOL_MAP_SIZE;
    int max = SYMBOL_MAP_SIZE << 1;
    while(max--)
    {
        if(index[i].index == -1)
            return -1;
        if(index[i].symbol == value)
            return index[i].index;
        i = (i + 1) % SYMBOL_MAP_SIZE;
    }
    fprintf(stderr, "ERROR get_symbol_index(%d)\n", value);
    exit(-1);
}

size_t set_symbol_index(int value, size_t ind, lookup_t * index)
{
    int i = value % SYMBOL_MAP_SIZE;
    int max = SYMBOL_MAP_SIZE << 1;
    while(max--)
    {
        if(index[i].index == -1)
        {
            index[i].index = ind;
            index[i].symbol = value;
            return index[i].index;
        }
        if(index[i].symbol == value)
        {
            index[i].index = ind;
            return index[i].index;
        }
        i = (i + 1) % SYMBOL_MAP_SIZE;
    }
    fprintf(stderr, "ERROR set_symbol_index(%d)\n", value);
    exit(-1);
}
int check_symbol_index(int value, lookup_t * index)
{
    int i = value % SYMBOL_MAP_SIZE;
    int max = SYMBOL_MAP_SIZE << 1;
    while(max--)
    {
        if(index[i].index == -1)
            return 0;
        if(index[i].symbol == value)
            return 1;
        i = (i + 1) % SYMBOL_MAP_SIZE;
    }
    return 0;
}
void clear_block_header(struct block_header header)
{
    free(header.index);
    free(header.symbol);
    free(header.freq);
    free(header.cumalative_freq);
    free(header.I_max);
}
struct block_header calculate_block_header(uint32_t * block, size_t block_size)
{
    struct block_header header;
    uint32_t * map = calloc(SYMBOL_MAP_SIZE, sizeof(uint32_t));
    lookup_t * sym_lookup = build_lookup();
    size_t i = 0;
    size_t ind = 0;
    size_t si;
    uint32_t max_symbol = 0;
    uint32_t cumalative_freq = 0;
    while(i<block_size){
        if (max_symbol < block[i])
            max_symbol = block[i];
        si = get_symbol_index(block[i], sym_lookup);
        if(si == -1)
        {
            si = set_symbol_index(block[i], ind, sym_lookup);
            ind++;
        }
        map[si] += 1;
        i++;
    }
    ind = 0;
    i = 0;
    while(i <= max_symbol){
        if(check_symbol_index(i, sym_lookup)){
            ind++;
        }
        i++;
    }
    header.no_symbols = ind;
    ind=0;
    i=0;
    header.index = sym_lookup;
    header.symbol = malloc(sizeof(uint32_t) * header.no_symbols);
    header.freq = malloc(sizeof(uint32_t) * header.no_symbols);
    header.cumalative_freq = malloc(sizeof(uint32_t) * header.no_symbols);
    header.I_max = malloc(sizeof(uint64_t) * header.no_symbols);
    while(i <= max_symbol){
        if(check_symbol_index(i, sym_lookup)){
            header.symbol[ind] = i;
            header.freq[ind] = map[get_symbol_index(i, sym_lookup)];
            header.cumalative_freq[ind] = cumalative_freq;
            header.I_max[ind] = (map[get_symbol_index(i, sym_lookup)] * B) - 1;
            cumalative_freq += map[get_symbol_index(i, sym_lookup)];
            set_symbol_index(i, ind, sym_lookup);
            ind++;
        }
        i++;
    }
    header.m = block_size;
    header.no_symbols = ind;
    header.block_len = 0;
    free(map);
    return header;
}











void bANS_decode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions, int flag)
{
    uint32_t no_blocks;
    uint32_t i = 0;
    struct reader * my_reader = initialise_reader(input_file);
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    no_blocks = vbyte_decode(metadata);
    while(i < no_blocks){
        process_decode_block(my_reader, output_file, my_prelude_functions);
        i++;
    }
}

void process_decode_block(struct reader * my_reader, FILE * output_file, struct prelude_functions * my_prelude_functions)
{
    size_t i = 0;
    uint64_t state;
    uint32_t * output;
    struct block_header header = read_block_header(&state, my_reader, my_prelude_functions);
    output = malloc(sizeof(uint32_t) * header.block_len);
    struct output_obj input;
    input.output = malloc(sizeof(unsigned char) * header.content_length);
    read_bytes(input.output, header.content_length, my_reader);
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
    uint32_t ls = header->freq[index];
    uint32_t bs = header->cumalative_freq[index];
    *output = header->symbol[index];
    *state = ls * (*state / header->m) + (*state % header->m) - bs;
}

struct block_header read_block_header(uint64_t * state, struct reader * my_reader, struct prelude_functions * my_prelude_functions)
{
    struct block_header header;
    size_t i = 0;
    size_t ind = 0;
    uint32_t cumalative_freq = 0;
    read_symbol_prelude(&(header.no_symbols), &(header.symbol), &(header.freq), state, &(header.content_length), my_reader, my_prelude_functions);
    header.cumalative_freq = malloc(sizeof(uint32_t) * header.no_symbols);
    header.symbol_state = malloc(sizeof(size_t) * BLOCK_SIZE);
    while(i < header.no_symbols){
        header.cumalative_freq[i] = cumalative_freq;
        cumalative_freq += header.freq[i];
        i++;
    }
    header.m = cumalative_freq;
    header.block_len = cumalative_freq;
    i = 0;
    while(i < header.block_len){
        if(ind < (header.no_symbols-1) && i >= header.cumalative_freq[ind + 1])
            ind += 1;
        header.symbol_state[i] = ind;
        i++;
    }
    return header;
}
