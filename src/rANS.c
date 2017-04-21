#include "rANS.h"
void rANS_encode(FILE * input_file, FILE * output_file, struct header header){
    struct preamble preamble = build_preamble(header);
    long int size, final_group_size, final_group, groups, i, c;
    uint symbol;
    uint * buffer;
    struct reverse_reader reader;
    uint value;
    uint64_t state;
    uint64_t ssstate;
    struct buffered_writer writer;
    fseek(input_file, 0L, SEEK_END);
    size = ftell(input_file);
    final_group = size % (BUFFER_SIZE * sizeof(uint));
    groups = size / (BUFFER_SIZE * sizeof(uint));
    buffer = malloc(sizeof(uint) * BUFFER_SIZE);
    writer.max_size = OUT_BUFFER_SIZE;
    writer.size = 0;
    writer.buffer = malloc(sizeof(unsigned char) * OUT_BUFFER_SIZE);
    writer.file = output_file;
    state = header.no_symbols;
    reader = get_reader(input_file);
    while(yield_uint(&reader, &value) != 0){
        ssstate = state;
        state = process_symbol(state, value, preamble, header, &writer);
    }
    writer_flush(&writer);
    fwrite(&state, sizeof(uint64_t), 1, writer.file);
    fflush(writer.file);
}
struct preamble build_preamble(struct header header){
    struct preamble preamble;
    uint64_t x, y, prev, c;
    preamble.symbol_state = malloc(sizeof(uint64_t) * header.no_symbols);
    preamble.cumalative_frequency = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    preamble.I_max = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    preamble.I_min = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    x = 0;
    prev = 0;
    c = 0;
    preamble.write_size = 1 << (8 * BYTES_TO_WRITE_OUT);
    while (x < header.no_unique_symbols){
        preamble.cumalative_frequency[x] = prev;
        prev = prev + header.symbol_frequencies[x];
        preamble.I_max[x] = (header.symbol_frequencies[x] * preamble.write_size) - 1;
        preamble.I_min[x] = header.symbol_frequencies[x];
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
    uint64_t m, x, ls, bs, first_comp, second_comp;
    uint64_t symbol = safe_get_symbol_index(input_symbol, &header);
    //printf("pre -STATE %ld\n", (long)state);
    while(state > preamble.I_max[symbol]){
        output = state % preamble.write_size;
        put(output, writer);
        state = state / preamble.write_size;
        //printf("mod -STATE %ld\n", (long)state);
    }
    m = header.no_symbols;
    x = state;

    ls = header.symbol_frequencies[symbol];
    bs = preamble.cumalative_frequency[symbol];
    first_comp = x / ls;
    first_comp = m * first_comp;
    second_comp = x % ls;
    //printf("post-STATE %ld\n", (long)(first_comp + bs + second_comp));
    return first_comp + bs + second_comp;
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
        fwrite(writer->buffer, sizeof(unsigned char), writer->size, writer->file);
        writer->size = 0;
    }
    fflush(writer->file);
}
void rANS_decode(FILE * input_file, FILE * output_file, struct header header, unsigned char verbose_flag){
    struct preamble preamble = build_preamble(header);
    size_t header_end, size, content_end, read_amount, current_read, i;
    struct decode_source source;
    unsigned char * buffer;
    unsigned char input;
    uint64_t state, symbol, current;
    struct buffered_uint_writer writer;
    header_end = ftell(input_file);
    fseek(input_file, 0, SEEK_END);
    size = ftell(input_file);
    content_end = size - sizeof(uint64_t);
    fseek(input_file, content_end, SEEK_SET);
    fread(&state, sizeof(uint64_t), 1, input_file);
    read_amount = content_end - header_end;
    buffer = malloc(sizeof(unsigned char) * BUFFER_SIZE);
    current = 0;
    writer.max_size = OUT_BUFFER_SIZE;
    writer.size = 0;
    writer.buffer = malloc(sizeof(uint) * OUT_BUFFER_SIZE);
    writer.file = output_file;
    source = get_decoder_source(input_file, header_end, content_end);
    while(current < header.no_symbols){
        symbol = preamble.symbol_state[state % header.no_symbols];
        state = calculate_state(&header, &preamble, symbol, state);
        current++;
        write_out((uint)header.symbols[symbol], &writer);
        while(state < header.no_symbols){
            input = yield_decoder_byte(&source);
            state = state * preamble.write_size + input;
            i++;
        }
    }
    write_flush(&writer);
}
void write_out(uint symbol, struct buffered_uint_writer * writer){
    writer->buffer[writer->size] = symbol;
    writer->size += 1;
    if(writer->size == writer->max_size){
        fwrite(writer->buffer, sizeof(uint), writer->size, writer->file);
        writer->size = 0;
    }
}
void write_flush(struct buffered_uint_writer * writer){
    if(writer->size > 0){
        fwrite(writer->buffer, sizeof(uint), writer->size, writer->file);
        writer->size = 0;
    }
    fflush(writer->file);
}
uint64_t get_symbol(struct preamble * preamble, uint64_t * state, struct header * header){
    uint64_t symbol, ls, x, m, bs;
    x = *state;
    m = header->no_symbols;
    symbol = preamble->symbol_state[x % m];
    bs = preamble->cumalative_frequency[symbol];
    ls = header->symbol_frequencies[symbol];
    *state =  ls * (x / m) + (x % m) - bs;
    return symbol;
}

uint64_t calculate_state(struct header * header, struct preamble * preamble, uint64_t symbol, uint64_t state){
    uint64_t ls, x, m, bs, result;
    ls = header->symbol_frequencies[symbol];
    //printf("ls  %ld\n", ls);
    x = state;
    //printf("x   %ld\n", x);
    m = header->no_symbols;
    //printf("m   %ld\n", m);
    bs = preamble->cumalative_frequency[symbol];
    //printf("bs  %ld\n", bs);
    result = (ls * (x / m)) + (x % m) - bs;
    //printf("sts %ld\n", result);
    //sleep(1);
    return result;
}