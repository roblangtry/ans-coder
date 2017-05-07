#include "tANS.h"
void tANS_encode(FILE * input_file, FILE * output_file, struct header header){
    struct t_preamble preamble = t_build_preamble(header, 0);
    struct reverse_reader reader;
    uint value;
    uint64_t state;
    struct t_buffered_writer writer;
    writer.max_size = OUT_BUFFER_SIZE;
    writer.size = 0;
    writer.current_byte = 0;
    writer.current_position = 0;
    writer.buffer = malloc(sizeof(unsigned char) * OUT_BUFFER_SIZE);
    writer.file = output_file;
    state = header.no_symbols;
    reader = get_reader(input_file);
    while(yield_uint(&reader, &value) != 0){
        state = t_process_symbol(state, value, preamble, header, &writer);
    }
    t_writer_flush(&writer);
    fwrite(&state, sizeof(uint64_t), 1, writer.file);
    fflush(writer.file);
}
struct t_preamble t_build_preamble(struct header header, unsigned char encode){
    struct t_preamble preamble;
    uint64_t i, x, y, prev;
    preamble.cumalative_frequency = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    if(encode == 0)
        preamble.I_max = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    if(encode == 1)
        preamble.aligned_frequency = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    x = 0;
    prev = 0;
    preamble.write_size = 2;
    preamble.bits_to_write = 1;
    preamble.I = header.no_symbols << 1;
    while (x < header.no_unique_symbols){
        preamble.cumalative_frequency[x] = prev;
        if(encode == 1)
            preamble.aligned_frequency[x] = header.symbol_frequencies[x] - preamble.cumalative_frequency[x];
        prev = prev + header.symbol_frequencies[x];
        if(encode == 0)
            preamble.I_max[x] = header.symbol_frequencies[x] << 1;
        x++;
    }
    if(encode == 1){
        preamble.sym_lut = malloc(sizeof(uint64_t) * header.no_symbols);
        preamble.state_lut = malloc(sizeof(uint64_t) * header.no_symbols);
        i = 0;
        y = 0;
        while(i < header.no_symbols){
            if(y != header.no_unique_symbols - 1){
                if(i == preamble.cumalative_frequency[y+1]){
                    y++;
                }
            }
            preamble.sym_lut[i] = y;
            preamble.state_lut[i] = preamble.aligned_frequency[i] + i;
            i++;
        }
    }
    return preamble;
}
uint64_t t_process_symbol(uint64_t state, uint input_symbol, struct t_preamble preamble, struct header header, struct t_buffered_writer * writer){
    unsigned char output;
    uint64_t m, x, ls, bs, first_comp, second_comp;
    uint64_t symbol = safe_get_symbol_index(input_symbol, &header);
    //printf("pre -STATE %ld\n", (long)state);
    while(state > preamble.I_max[symbol]){
        output = state % preamble.write_size;
        t_put(output, writer);
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
void t_put(unsigned char byte, struct t_buffered_writer * writer){
    writer->current_byte = (writer->current_byte << 1) + byte;
    writer->current_position += 1;
    if(writer->current_position == 8){
        writer->buffer[writer->size] = writer->current_byte;
        writer->size += 1;
        writer->current_byte = 0;
        writer->current_position = 0;
        if(writer->size == writer->max_size){
            fwrite(writer->buffer, sizeof(unsigned char), writer->max_size, writer->file);
            writer->size = 0;
        }
    }
}
void t_writer_flush(struct t_buffered_writer * writer){
    if(writer->size > 0){
        fwrite(writer->buffer, sizeof(unsigned char), writer->size, writer->file);
        writer->size = 0;
    }
    fwrite(&writer->current_byte, sizeof(unsigned char), 1, writer->file);
    fwrite(&writer->current_position, sizeof(unsigned char), 1, writer->file);
    fflush(writer->file);
}
void tANS_decode(FILE * input_file, FILE * output_file, struct header header, unsigned char verbose_flag){
    struct t_preamble preamble = t_build_preamble(header, 1);
    size_t header_end, size, content_end;
    struct decode_source source;
    unsigned char input;
    uint64_t state, symbol, current;
    struct t_buffered_uint_writer writer;
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
        current++;
        if(state >= preamble.I){
            symbol = preamble.sym_lut[state % header.no_symbols];
            t_write_out(header.symbols[symbol], &writer);
            state = header.symbol_frequencies[symbol] * (state / header.no_symbols) + (state % header.no_symbols) - preamble.cumalative_frequency[symbol];
        }else{
            t_write_out(header.symbols[preamble.sym_lut[state - header.no_symbols]], &writer);
            state = preamble.state_lut[state - header.no_symbols];
        }
        while(state < header.no_symbols){
            yield_decoder_bit(&source, &input);
            if(input == 3)
                break;
            state = (state << preamble.bits_to_write) + input;
        }
    }
    t_write_flush(&writer);
}
void t_write_out(uint symbol, struct t_buffered_uint_writer * writer){
    writer->buffer[writer->size] = symbol;
    writer->size += 1;
    if(writer->size == writer->max_size){
        fwrite(writer->buffer, sizeof(uint), writer->size, writer->file);
        writer->size = 0;
    }
}
void t_write_flush(struct t_buffered_uint_writer * writer){
    if(writer->size > 0){
        fwrite(writer->buffer, sizeof(uint), writer->size, writer->file);
        writer->size = 0;
    }
    fflush(writer->file);
}

//uint64_t t_calculate_state(struct header * header, struct t_preamble * preamble, uint64_t * symbol, uint64_t state){
//    uint64_t ls, x, m, bs, result, w, x_mod_m, x_div_m;
//    x = state;
//    m = header->no_symbols;
//    x_mod_m = x % m;
//    x_div_m = x / m;
//    *symbol = preamble->symbol_state[x_mod_m];
//    ls = header->symbol_frequencies[*symbol];
//    bs = preamble->cumalative_frequency[*symbol];
//    if (x < preamble->I){
//        w = x_div_m - 1;
//        result = (preamble->ls_lut[*symbol][w]) + (x_mod_m) - preamble->cumalative_frequency[*symbol];
//    }
//    else {
//        result = (ls * x_div_m) + (x % m) - preamble->cumalative_frequency[*symbol];
//    }
//    return result;
//}//