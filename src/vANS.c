#include "vANS.h"


void write_vector(struct prelude_code_data * metadata, vector_t * vector)
{
    uint i;
    elias_encode(metadata, vector->length);
    for(i = 0; i < vector->length; i++)
    {
        write_bits(vector->data[i], 32, metadata->bit_writer_ptr);
    }
    elias_flush(metadata);
}

vector_t * read_vector(struct prelude_code_data * metadata)
{
    uint i;
    vector_t * vector = mymalloc(sizeof(vector_t));
    vector->length = elias_decode(metadata);
    vector->data = mymalloc(sizeof(uint32_t) * vector->length);
    for(i = 0; i < vector->length; i++)
    {
        read_bits(32, metadata->bit_reader_ptr);
    }
    return vector;
}

void add_to_vector(vector_t * vector, uint32_t number)
{
    uint32_t container_number = (number - 1) / 32;
    uint32_t offset = (number-1) % 32;
    uint32_t i;

    if(container_number >= vector->length){
        vector->data = realloc(vector->data, (container_number+1) * sizeof(uint32_t));
        for(i = vector->length; i <= container_number; i++)
            vector->data[i] = 0;
        vector->length = container_number+1;
    }
    vector->data[container_number] = vector->data[container_number] | (1 << offset);
}

unsigned char check_vector(vector_t * vector, uint32_t number)
{
    uint32_t container_number = (number - 1) / 32;
    uint32_t offset = (number-1) % 32;
    return vector->data[container_number] & (1 << offset);
}

vector_t * get_blank_vector()
{
    vector_t * vector = mymalloc(sizeof(vector_t));
    vector->data = mymalloc(sizeof(uint32_t));
    vector->data[0] = 0;
    vector->length = 1;
    return vector;
}

uint32_t iterate_vector(uint32_t hot_start, vector_t * vector)
{
    uint32_t value = hot_start + 1;
    while(!check_vector(vector, value)) value++;
    return value;
}

void vANS_encode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions)
{
    uint32_t block[BLOCK_SIZE];
    size_t size;
    struct writer * my_writer = initialise_writer(output_file);
    write_vector_meta_header(input_file, my_writer);
    size = fread(block, sizeof(uint32_t), BLOCK_SIZE, input_file);
    while(size > 0){
        process_encode_vector_block(block, size, my_writer, my_prelude_functions);
        size = fread(block, sizeof(uint32_t), BLOCK_SIZE, input_file);
    }
    flush_writer(my_writer);
}

void process_encode_vector_block(uint32_t * block, size_t block_size, struct writer * my_writer, struct prelude_functions * my_prelude_functions){
    uint64_t state = block_size;
    struct block_header header;
    header = calculate_block_header(block, block_size, VECTOR_METHOD);
    size_t i = block_size;
    struct output_obj output = get_output_obj(NULL);
    while(i > 0){
        i--;
        process_encode(block[i], &state, &header, &output);
    }
    write_vector_block(state, &header, &output, my_writer, my_prelude_functions);
}
void write_vector_block(uint64_t state, struct block_header * header, struct output_obj * output, struct writer * my_writer, struct prelude_functions * my_prelude_functions)
{
    size_t head = output->head;
    write_vector_symbol_prelude(header->symbol, header->freq, &header->no_symbols, &state, &head, my_writer, my_prelude_functions);
    write_bytes(output->output, output->head, my_writer);
}
vector_t * prepare_common_vector(FILE * input_file, struct writer * my_writer)
{
    unsigned char current[SYMBOL_MAP_SIZE];
    unsigned char total[SYMBOL_MAP_SIZE];
    uint32_t block[BLOCK_SIZE];
    uint i;
    uint max;
    size_t size;
    uint32_t count = 0;
    vector_t * vector = get_blank_vector();
    for(i = 0; i<SYMBOL_MAP_SIZE; i++){
        current[i] = 0;
        total[i] = 0;
    }
    size = fread(block, sizeof(uint32_t), BLOCK_SIZE, input_file);
    while(size > 0){
        count++;
        max = 0;
        for(i = 0; i<size; i++){
            current[block[i] - 1] = 1;
            if(block[i]>max) max = block[i];
        }
        for(i=max;i+1>0;i--)
        {
            if(current[i])
            {
                total[i]++;
                current[i] = 0;
            }
        }
        size = fread(block, sizeof(uint32_t), BLOCK_SIZE, input_file);
    }
    for(i=0;i<SYMBOL_MAP_SIZE;i++)
    {
        if(total[i] >= count/10*8) add_to_vector(vector, i);
    }
    fseek(input_file, 0, SEEK_SET);
    return vector;
}

void write_vector_meta_header(FILE * input_file, struct writer * my_writer)
{
    size_t size;
    uint32_t no_blocks;
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    // vector_t * common_vector;
    fseek(input_file, 0, SEEK_END);
    size = ftell(input_file);
    no_blocks = (size / sizeof(uint32_t)) / BLOCK_SIZE;
    if(((size / sizeof(uint32_t)) % BLOCK_SIZE) > 0)
        no_blocks += 1;
    fseek(input_file, 0, SEEK_SET);
    //common_vector = prepare_common_vector(input_file, my_writer);
    elias_encode(metadata, no_blocks);
    //write_vector(metadata, common_vector);
}

void vANS_decode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions)
{
    uint32_t no_blocks;
    uint32_t i = 0;
    struct reader * my_reader = initialise_reader(input_file);
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    // vector_t * common_vector;
    no_blocks = elias_decode(metadata);
    //common_vector = read_vector(metadata);
    while(i < no_blocks){
        process_vector_decode_block(my_reader, output_file, my_prelude_functions);
        i++;
    }
}

void process_vector_decode_block(struct reader * my_reader, FILE * output_file, struct prelude_functions * my_prelude_functions)
{
    size_t i = 0;
    uint64_t state;
    uint32_t * output;
    struct block_header header = read_vector_block_header(&state, my_reader, my_prelude_functions);
    output = mymalloc(sizeof(uint32_t) * header.block_len);
    struct output_obj input;
    input.output = mymalloc(sizeof(unsigned char) * header.content_length);
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

struct block_header read_vector_block_header(uint64_t * state, struct reader * my_reader, struct prelude_functions * my_prelude_functions)
{
    struct block_header header;
    size_t i = 0;
    size_t ind = 0;
    uint32_t cumalative_freq = 0;
    read_vector_symbol_prelude(&(header.no_symbols), &(header.symbol), &(header.freq), state, &(header.content_length), my_reader, my_prelude_functions);
    header.cumalative_freq = mymalloc(sizeof(uint32_t) * header.no_symbols);
    header.symbol_state = mymalloc(sizeof(size_t) * BLOCK_SIZE);
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




void write_vector_symbol_prelude(uint32_t * symbols, uint32_t * symbol_frequencies, size_t * no_symbols, uint64_t * state, size_t * content_length, struct writer * my_writer, struct prelude_functions * my_prelude_functions)
{
    struct prelude_code_data * metadata = prepare_metadata(NULL, my_writer, 0);
    vector_t * vector = get_blank_vector();
    elias_encode(metadata, *state);
    elias_encode(metadata, *no_symbols);
    elias_encode(metadata, *content_length);

    uint64_t i = 0;
    while(i < *no_symbols){
        add_to_vector(vector, symbols[i]);
        i++;
    }
    write_vector(metadata, vector);
    while(i < *no_symbols){
        elias_encode(metadata, symbol_frequencies[i]);
        i++;
    }
    elias_flush(metadata);
}
void read_vector_symbol_prelude(size_t * no_symbols, uint32_t ** symbols, uint32_t ** symbol_frequencies, uint64_t * state, size_t * content_length, struct reader * my_reader, struct prelude_functions * my_prelude_functions)
{
    uint64_t i = 0;
    uint64_t last_symbol = 0;
    vector_t * vector;
    struct prelude_code_data * metadata = prepare_metadata(my_reader, NULL, 0);
    *state = elias_decode(metadata);
    *no_symbols = elias_decode(metadata);
    *content_length = elias_decode(metadata);
    vector = read_vector(metadata);

    *symbols = mymalloc(sizeof(uint32_t) * (*no_symbols));
    *symbol_frequencies = mymalloc(sizeof(uint32_t) * (*no_symbols));
    while(i < *no_symbols){
        (*symbols)[i] = iterate_vector(last_symbol, vector);
        //fprintf(stderr, "S %d\n", last_symbol);
        // sleep(1);
        last_symbol = (*symbols)[i];
        (*symbol_frequencies)[i] = elias_decode(metadata);
        i++;
    }
}