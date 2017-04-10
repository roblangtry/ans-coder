#include "rANS.h"
rANS_encode(FILE * input_file, FILE * output_file, struct header header){
    struct preamble preamble = build_preamble(header);
    long int size, final_group, groups, i, c;
    uint symbol;
    uint * buffer;
    uint64_t state;
    struct buffered_writer writer;
    fseek(input_file, 0L, SEEK_END);
    size = ftell(input_file);
    final_group = size % (BUFFER_SIZE * sizeof(uint));
    groups = size / (BUFFER_SIZE * sizeof(uint));
    buffer = malloc(sizeof(uint) * BUFFER_SIZE);
    writer.max_size = sizeof(uint) * BUFFER_SIZE;
    writer.size = 0;
    writer.buffer = malloc(sizeof(uint) * BUFFER_SIZE);
    writer.file = output_file;
    state = header.no_symbols;
    if(final_group > 0){
        fseek(input_file, groups * BUFFER_SIZE, SEEK_SET);
        fread(buffer, sizeof(uint), final_group, input_file);
        c = 1;
        while(c <= final_group){
            symbol = buffer[final_group - c];
            c++;
            state = process_symbol(state, symbol, preamble, header, &writer);
        }
    }
    i = 1;
    while(i <= groups){
        fseek(input_file, (groups - i) * BUFFER_SIZE, SEEK_SET);
        fread(buffer, sizeof(uint), BUFFER_SIZE, input_file);
        c = 1;
        while(c <= BUFFER_SIZE){
            symbol = buffer[BUFFER_SIZE - c];
            c++;
            state = process_symbol(state, symbol, preamble, header, &writer);
        }
        i++;
    }
    writer_flush(&writer);
}
struct preamble build_preamble(struct header header){
    struct preamble preamble;
    uint64_t x, y, prev, c;
    preamble.symbol_state = malloc(sizeof(uint64_t) * header.no_symbols);
    preamble.cumalative_frequency = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    preamble.I_max = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    x = 0;
    prev = 0;
    c = 0;
    preamble.write_size = 1 << (8 * BYTES_TO_WRITE_OUT);
    while (x < header.no_unique_symbols){
        prev = prev + header.symbol_frequencies[x];
        preamble.cumalative_frequency[x] = prev;
        preamble.I_max[x] = (header.symbol_frequencies[x] * preamble.write_size) - 1;
        y = 0;
        while(y < header.symbol_frequencies[x]){
            preamble.symbol_state[c] = x;
            c++;
            y++;
        }
        x++;
    }
    return preamble;
}
uint64_t process_symbol(uint64_t state, uint input_symbol, struct preamble preamble, struct header header, struct buffered_writer * writer){
    unsigned char output;
    uint64_t symbol = get_symbol_index(input_symbol, &header);
    while(state > preamble.I_max[symbol]){
        output = state % preamble.write_size;
        put(output, writer);
        state = state / preamble.write_size;
    }
    return header.no_symbols * (state / header.symbol_frequencies[symbol]) + preamble.cumalative_frequency[symbol] + (state % header.symbol_frequencies[symbol]);
}
void put(unsigned char byte, struct buffered_writer * writer){
    writer->buffer[writer->size] = byte;
    writer->size += 1;
    if(writer->size == writer->max_size){
        fwrite(writer->buffer, sizeof(unsigned char), writer->max_size, writer->file);
        writer->size = 0;
    }
}
void writer_flush(struct buffered_writer * writer){
    if(writer->size > 0){
        fwrite(writer->buffer, sizeof(unsigned char), writer->max_size, writer->file);
        writer->size = 0;
    }
    fflush(writer->file);
}