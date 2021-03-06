#include "bANS.h"


void bANS_encode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions, int flag)
{
    uint32_t block[BLOCK_SIZE];
    size_t size;
    struct writer * my_writer = initialise_writer(output_file);
    write_meta_header(input_file, my_writer);
    size = fread(block, sizeof(uint32_t), BLOCK_SIZE, input_file);
    while(size > 0){
        process_encode_block(block, size, my_writer, my_prelude_functions, flag);
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
    free_metadata(metadata);
}
void standard_encode(uint32_t symbol, uint64_t * state, struct block_header * header, struct output_obj * output)
{
    process_encode(symbol, state, header, output);
}
void msb_encode(uint32_t symbol, uint64_t * state, struct block_header * header, struct output_obj * output)
{
    uint32_t out_symbol = 0;
    if(symbol <= 256) out_symbol = symbol;
    else if(symbol <= 65536) out_symbol = (symbol >> 8) + 256;
    else if(symbol <= 16777216) out_symbol = (symbol >> 16) + 512;
    process_encode(out_symbol, state, header, output);
}
void split_encode(uint32_t symbol, uint64_t * state, struct block_header * header, struct output_obj * output)
{
    uint32_t value = symbol;
    uint32_t out;
    uint32_t V[8];
    int index = 0;
    while(value)
    {
        out = value % (1 << (SPLIT_LENGTH - 1));
        value = value >> (SPLIT_LENGTH - 1);
        if(value)
            out += (1 << (SPLIT_LENGTH - 1));
        V[index] = out;
        index++;
    }
    index--;
    while(index){
        process_encode(V[index], state, header, output);
        index--;
    }
    process_encode(V[index], state, header, output);

}
void process_encode_block(uint32_t * block, size_t block_size, struct writer * my_writer, struct prelude_functions * my_prelude_functions, int flag){
    uint64_t state = block_size;
    uint32_t vbyte;
    unsigned char byte;
    struct block_header header = calculate_block_header(block, block_size, flag);
    size_t i = block_size;
    struct output_obj output = get_output_obj(NULL);
    while(i > 0){
        // if(i >= 510 && i <= 520) printf("[%d] = %d\n", i , block[i]);
        i--;
        if(flag == SPLIT_METHOD) split_encode(block[i], &state, &header, &output);
        if(flag == MSB_METHOD) msb_encode(block[i], &state, &header, &output);
        else standard_encode(block[i], &state, &header, &output);
    }
    write_block(state, &header, &output, my_writer, my_prelude_functions);
    if(flag == MSB_METHOD)
    {
        i = 0;
        while(i < block_size)
        {
            if(block[i] > 256 && block[i] <= 65536)
            {
                vbyte = block[i] % 256;
                byte = vbyte;
                write_byte(byte, my_writer);
            }
            else if(block[i] > 65536 && block[i] <= 16777216)
            {
                vbyte = (block[i] >> 8) % 256;
                byte = vbyte;
                write_byte(byte, my_writer);
                vbyte = block[i] % 256;
                byte = vbyte;
                write_byte(byte, my_writer);
            }
            i++;
        }
    }
    clear_block_header(header);
    free_output_obj(output);
}

void process_encode(uint32_t symbol, uint64_t * state, struct block_header * header, struct output_obj * output)
{
    size_t index = get_symbol_index(symbol, header->index);
    uint32_t ls = header->freq[index];
    uint32_t bs = header->cumalative_freq[index];
    uint32_t Is = (ls * (1 << Bbits)) - 1;
    while(*state > Is){
        write_output(state, output);
    }
    *state = header->m * (*state / ls) + bs + (*state % ls);
    header->block_len += 1;
}


void write_output(uint64_t * state, struct output_obj * output)
{
    output->output[output->head] = *state % (1 << Bbits);
    *state = *state >> Bbits;
    output->head += 1;
}

void write_block(uint64_t state, struct block_header * header, struct output_obj * output, struct writer * my_writer, struct prelude_functions * my_prelude_functions)
{
    size_t head = output->head;
    write_symbol_prelude(header->symbol, header->freq, &header->no_symbols, &state, &head, my_writer, my_prelude_functions);
    write_bytes(output->output, output->head, my_writer);
}
void free_output_obj(struct output_obj obj)
{
    FREE(obj.output);
}
struct output_obj get_output_obj(FILE * output_file)
{
    struct output_obj output;
    output.head = 0;
    output.file = output_file;
    output.output = mymalloc(sizeof(unsigned char) * (BLOCK_SIZE << 2));
    return output;
}
lookup_t * build_lookup()
{
    int i;
    lookup_t * index = (lookup_t *)mymalloc(sizeof(lookup_t) * SYMBOL_MAP_SIZE);
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
    if(header.index != NULL) FREE(header.index);
    if(header.symbol != NULL) FREE(header.symbol);
    if(header.symbol_state != NULL) FREE(header.symbol_state);
    if(header.freq != NULL) FREE(header.freq);
    if(header.cumalative_freq != NULL) FREE(header.cumalative_freq);
}
uint32_t add_symbol_to_index(uint32_t symbol, uint32_t max_symbol, size_t * ind, uint32_t * map, lookup_t * sym_lookup)
{
    size_t si;
    if (max_symbol < symbol)
        max_symbol = symbol;
    si = get_symbol_index(symbol, sym_lookup);
    if(si == -1)
    {
        si = set_symbol_index(symbol, *ind, sym_lookup);
        (*ind)++;
    }
    map[si] += 1;
    return max_symbol;
}
struct block_header calculate_block_header(uint32_t * block, size_t block_size, int flag)
{
    struct block_header header;
    header.index = NULL;
    header.symbol_state = NULL;
    header.symbol = NULL;
    header.freq = NULL;
    header.cumalative_freq = NULL;
    uint32_t * map = calloc(BLOCK_SIZE, sizeof(uint32_t));
    lookup_t * sym_lookup = build_lookup();
    size_t i = 0;
    size_t ind = 0;
    uint32_t V=0, O=0;
    uint32_t max_symbol = 0;
    uint32_t cumalative_freq = 0;
    uint64_t c = 0;
    while(i<block_size){
        if(flag == SPLIT_METHOD)
        {
            V = block[i];
            while(V > 0)
            {
                c++;
                O = V % (1 << (SPLIT_LENGTH - 1));
                V = V >> (SPLIT_LENGTH - 1);
                if(V)
                    O += (1 << (SPLIT_LENGTH - 1));
                max_symbol = add_symbol_to_index(O, max_symbol, &ind, map, sym_lookup);
            }
        }
        else if(flag == MSB_METHOD)
        {
            V = block[i];
            if(V <= 256) O = V;
            else if(V <= 65536) O = (V >> 8) + 256;
            else if(V <= 16777216) O = (V >> 16) + 512;
            max_symbol = add_symbol_to_index(O, max_symbol, &ind, map, sym_lookup);
        }
        else
        {
            max_symbol = add_symbol_to_index(block[i], max_symbol, &ind, map, sym_lookup);
        }
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
    header.symbol = mymalloc(sizeof(uint32_t) * header.no_symbols);
    header.freq = mymalloc(sizeof(uint32_t) * header.no_symbols);
    header.cumalative_freq = mymalloc(sizeof(uint32_t) * header.no_symbols);
    while(i <= max_symbol){
        if(check_symbol_index(i, sym_lookup)){
            header.symbol[ind] = i;
            header.freq[ind] = map[get_symbol_index(i, sym_lookup)];
            header.cumalative_freq[ind] = cumalative_freq;
            cumalative_freq += map[get_symbol_index(i, sym_lookup)];
            set_symbol_index(i, ind, sym_lookup);
            ind++;
        }
        i++;
    }
    header.m = cumalative_freq;
    header.no_symbols = ind;
    header.block_len = 0;
    FREE(map);
    return header;
}











void bANS_decode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions, int flag)
{
    uint32_t no_blocks;
    uint32_t i = 0;
    struct reader * my_reader = initialise_reader(input_file);
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    no_blocks = vbyte_decode(metadata);
    free_metadata(metadata);
    while(i < no_blocks){
        process_decode_block(my_reader, output_file, my_prelude_functions,flag);
        i++;
    }
}
void standard_decode(uint64_t * state, struct block_header * header, uint32_t * output, struct output_obj * input)
{
    process_decode(state, header, output);
    while(*state < header->m && input->head >= 0){
        if(input->head >= 0)
        {
            input->head -= 1;
            *state = (*state << Bbits) + input->output[input->head];
        }
    }
}
void split_decode(uint64_t * state, struct block_header * header, uint32_t * output, struct output_obj * input)
{
    uint32_t V = 0;
    uint32_t I = (1 << (SPLIT_LENGTH - 1));
    uint32_t R = 0;
    while(I >= (1 << (SPLIT_LENGTH - 1)))
    {
        standard_decode(state, header, &I, input);
        V += ((I % (1 << (SPLIT_LENGTH - 1))) << ((SPLIT_LENGTH - 1) * R));
        R++;
    }
    *output = V;
}
void reassess_len(struct block_header * header, int flag)
{
    if(flag == SPLIT_METHOD)
    {
        int i = 0;
        int bl = 0;
        int ns = header->no_symbols;
        while(i < ns && (header->symbol[i] < (1 << (SPLIT_LENGTH - 1))))
        {
            bl += header->freq[i];
            i++;
        }
        header->block_len = bl;
    }
}
void process_decode_block(
    struct reader * my_reader,
    FILE * output_file,
    struct prelude_functions * my_prelude_functions,
    int flag)
{
    size_t i = 0;
    uint64_t state;
    unsigned char byte;
    uint32_t * output;
    struct block_header header = read_block_header(&state, my_reader, my_prelude_functions);
    if(flag == SPLIT_METHOD) reassess_len(&header, flag);
    output = mymalloc(sizeof(uint32_t) * header.m);
    struct output_obj input;
    input.output = mymalloc(sizeof(unsigned char) * header.content_length);
    read_bytes(input.output, header.content_length, my_reader);
    input.head = header.content_length;
    while(i < header.block_len){
        if(flag == SPLIT_METHOD) split_decode(&state, &header, &output[i], &input);
        else standard_decode(&state, &header, &output[i], &input);
        i++;
    }
    if(flag == MSB_METHOD)
    {
        i = 0;
        // for(i=0;i<20;i++){
        //     read_byte(&byte, my_reader);
        //     // printf("(%d)\n", byte);
        // }
        while(i < header.block_len)
        {

            // if(i >= 510 && i <= 520) printf("[%d] = %d\n", i , output[i]);
            if(output[i] > 256 && output[i] <= 512)
            {
                // if(i >= 510 && i <= 520) printf("[%d] = %d | ", i, output[i]);
                output[i] -= 256;
                // if(i >= 510 && i <= 520) printf("[%d] = %d | ", i, output[i]);
                read_byte(&byte, my_reader);
                // if(i >= 510 && i <= 520) printf("(%d)", byte);
                output[i] = (output[i]<<8) + byte;
                // if(i >= 510 && i <= 520) printf(" [%d] = %d\n", i, output[i]);
                //read_byte(&byte, my_reader);
                // sleep(1);
            }
            else if(output[i] > 512)
            {
                output[i] -= 512;
                read_byte(&byte, my_reader);
                output[i] = (output[i]<<8) + byte;
                read_byte(&byte, my_reader);
                output[i] = (output[i]<<8) + byte;
            }
            i++;
        }
    }
    // for(int i = 510; i <= 520; i++)
    // {
    //     printf("[%d] = %d\n", i , output[i]);
    // }
    fwrite(output, sizeof(uint32_t), header.block_len, output_file);
    FREE(output);
    FREE(input.output);
    clear_block_header(header);
}

void process_decode(uint64_t * state, struct block_header * header, uint32_t * output)
{
    size_t index = header->symbol_state[*state % header->m];
    uint32_t ls = header->freq[index];
    uint32_t bs = header->cumalative_freq[index];
    *output = header->symbol[index];
    *state = ls * (*state / header->m) + (*state % header->m) - bs;
}

struct block_header read_block_header(uint64_t * state, struct reader * my_reader, struct prelude_functions * my_prelude_functions)
{
    struct block_header header;
    header.index = NULL;
    header.symbol_state = NULL;
    header.symbol = NULL;
    header.freq = NULL;
    header.cumalative_freq = NULL;
    size_t i = 0;
    size_t ind = 0;
    uint32_t cumalative_freq = 0;
    read_symbol_prelude(&(header.no_symbols), &(header.symbol), &(header.freq), state, &(header.content_length), my_reader, my_prelude_functions);
    header.cumalative_freq = mymalloc(sizeof(uint32_t) * header.no_symbols);
    while(i < header.no_symbols){
        header.cumalative_freq[i] = cumalative_freq;
        cumalative_freq += header.freq[i];
        i++;
    }
    header.m = cumalative_freq;
    header.symbol_state = mymalloc(sizeof(size_t) * header.m);
    header.block_len = cumalative_freq;
    i = 0;
    while(i < header.m){
        if(ind < (header.no_symbols-1) && i >= header.cumalative_freq[ind + 1])
            ind += 1;
        header.symbol_state[i] = ind;
        i++;
    }
    return header;
}
