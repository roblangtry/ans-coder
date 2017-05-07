#include "rANS.h"
void rANS_encode(FILE * input_file, FILE * output_file, struct header header){
    struct preamble preamble = build_preamble(header);
    struct reverse_reader reader;
    uint value;
    uint64_t state;
    uint64_t n;
    struct buffered_writer writer;
    fseek(input_file, 0L, SEEK_END);
    writer.max_size = OUT_BUFFER_SIZE;
    writer.size = 0;
    writer.buffer = malloc(sizeof(unsigned char) * OUT_BUFFER_SIZE);
    writer.file = output_file;
    writer.byte = 0;
    writer.len = 0;
    state = header.no_symbols;
    reader = get_reader(input_file);
    n = 0;
    while(yield_uint(&reader, &value) != 0){
        state = process_symbol(state, value, preamble, header, &writer);
        n++;
    }
    bit_writer_flush(&writer);
    fwrite(&state, sizeof(uint64_t), 1, writer.file);
    fflush(writer.file);
}
struct preamble build_preamble(struct header header){
    struct preamble preamble;
    uint64_t i, x, y, prev, c;
    size_t lut_size;
    preamble.symbol_state = malloc(sizeof(uint64_t) * header.no_symbols);
    preamble.cumalative_frequency = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    preamble.I_max = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    preamble.I_min = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    x = 0;
    prev = 0;
    c = 0;
    preamble.write_size = 1 << (BITS_TO_WRITE_OUT);
    preamble.bits_to_write = BITS_TO_WRITE_OUT;
    preamble.I = header.no_symbols << preamble.bits_to_write - 1;
    preamble.ls_lut = malloc(sizeof(uint64_t *) * header.no_unique_symbols);
    lut_size = sizeof(uint64_t) * preamble.write_size;
    while (x < header.no_unique_symbols){
        preamble.ls_lut[x] = malloc(lut_size);
        i = 1;
        preamble.ls_lut[x][0] = header.symbol_frequencies[x];
        while(i < preamble.write_size){
            preamble.ls_lut[x][i] = preamble.ls_lut[x][i - 1] + header.symbol_frequencies[x];
            i++;
        }
        preamble.cumalative_frequency[x] = prev;
        prev = prev + header.symbol_frequencies[x];
        preamble.I_max[x] = (header.symbol_frequencies[x] << preamble.bits_to_write) - 1;
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
        state = state >> preamble.bits_to_write;
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
void put(unsigned char bit, struct buffered_writer * writer){
    writer->byte = (writer->byte << 1) + bit;
    writer->len += 1;
    if(writer->len == 8){
        writer->buffer[writer->size] = writer->byte;
        writer->size += 1;
        if(writer->size == writer->max_size){
            fwrite(writer->buffer, sizeof(unsigned char), writer->max_size, writer->file);
            writer->size = 0;
        }
        writer->len = 0;
        writer->byte = 0;
    }
}
void bit_writer_flush(struct buffered_writer * writer){
    if(writer->size > 0){
        fwrite(writer->buffer, sizeof(unsigned char), writer->size, writer->file);
        writer->size = 0;
    }
    fwrite(&writer->byte, sizeof(unsigned char), 1, writer->file);
    fwrite(&writer->len, sizeof(unsigned char), 1, writer->file);
    writer->len = 0;
    writer->byte = 0;
    fflush(writer->file);
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
    size_t header_end, size, content_end,  i;
    struct decode_source source;
    unsigned char input;
    uint64_t state, symbol, current;
    struct buffered_uint_writer writer;
    header_end = ftell(input_file);
    fseek(input_file, 0, SEEK_END);
    size = ftell(input_file);
    content_end = size - sizeof(uint64_t);
    fseek(input_file, content_end, SEEK_SET);
    fread(&state, sizeof(uint64_t), 1, input_file);
    current = 0;
    writer.max_size = OUT_BUFFER_SIZE;
    writer.size = 0;
    writer.buffer = malloc(sizeof(uint) * OUT_BUFFER_SIZE);
    writer.file = output_file;
    source = get_decoder_source(input_file, header_end, content_end);
    while(current < header.no_symbols){
        state = calculate_state(&header, &preamble, &symbol, state);
        current++;
        write_out((uint)header.symbols[symbol], &writer);
        while(state < header.no_symbols){
            input = yield_decoder_bit(&source);
            if(input == 3){
                break;
            }
            else{
                state = (state << preamble.bits_to_write) + input;
            }
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

uint64_t calculate_state(struct header * header, struct preamble * preamble, uint64_t * symbol, uint64_t state){
    uint64_t ls, bs, x, m,  result, w, x_mod_m, x_div_m;
    x = state;
    m = header->no_symbols;
    x_mod_m = x % m;
    x_div_m = x / m;
    *symbol = preamble->symbol_state[x_mod_m];
    ls = header->symbol_frequencies[*symbol];
    bs = preamble->cumalative_frequency[*symbol];
    result = (ls * x_div_m) + (x_mod_m) - bs;
    return result;
}