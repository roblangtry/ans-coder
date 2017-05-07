#include "reverse_reader.h"
unsigned char yield_uint(struct reverse_reader * reader, unsigned int * value){
    if(reader->current >= reader->buffer_size){
        if(reader->stop == 1){
            return 0;
        } else {
            if(reader->buffer_start < (READER_BUFFER * sizeof(uint))){
                reader->buffer_size = reader->buffer_start / sizeof(uint);
                reader->buffer_start = 0;
                reader->stop = 1;
            } else {
                reader->buffer_start = reader->buffer_start - (READER_BUFFER * sizeof(uint));
                reader->buffer_size = READER_BUFFER;
            }
            fseek(reader->file, reader->buffer_start, SEEK_SET);
            reader->buffer_size = fread(reader->buffer, sizeof(uint), reader->buffer_size, reader->file);
            reader->current = 0;
        }
    }
    *value = reader->buffer[reader->buffer_size - reader->current - 1];
    reader->current += 1;
    return 1;
}
struct reverse_reader get_reader(FILE * file){
    struct reverse_reader reader;
    reader.file = file;
    fseek(file, 0L, SEEK_END);
    reader.size = ftell(file);
    reader.current = 0;
    reader.buffer_start = reader.size;
    reader.buffer_size = 0;
    reader.buffer = malloc(sizeof(uint) * READER_BUFFER);
    reader.stop = 0;
    if (reader.size == 0)
        reader.stop = 1;
    return reader;
}







struct decode_source get_decoder_source(FILE * file, size_t start, size_t end){
    unsigned char value;
    struct decode_source source;
    source.file = file;
    source.start = 1;
    source.buffer_size = 0;
    source.position = 0;
    source.content_start = start;
    source.buffer_start = end;
    source.current = 0;
    source.stop = 0;
    source.buffer = malloc(sizeof(unsigned char) * READER_BUFFER);
    if(source.start == 1){
        source.buffer_start -= (sizeof(unsigned char));
        fseek(source.file, source.buffer_start, SEEK_SET);
        fread(&value, sizeof(unsigned char), 1, source.file);
        source.position = value;
        if(source.buffer_start < (READER_BUFFER + source.content_start)){
            source.buffer_size = source.buffer_start - source.content_start;
            source.buffer_start = source.content_start;
            source.stop = 1;
        } else {
            source.buffer_start = source.buffer_start - (READER_BUFFER);
            source.buffer_size = READER_BUFFER;
        }
        fseek(source.file, source.buffer_start, SEEK_SET);
        source.buffer_size = fread(source.buffer, sizeof(unsigned char), source.buffer_size, source.file);
        source.current = 0;
        source.start = 0;
        source.current_byte = source.buffer[source.buffer_size - source.current - 1];
    }
    return source;
}
unsigned char yield_decoder_byte(struct decode_source * source){
    unsigned char value;
    if(source->current >= source->buffer_size){
        if(source->stop == 1){
            return 0;
        } else {
            if(source->buffer_start < (READER_BUFFER + source->content_start)){
                source->buffer_size = source->buffer_start - source->content_start;
                source->buffer_start = source->content_start;
                source->stop = 1;
            } else {
                source->buffer_start = source->buffer_start - (READER_BUFFER);
                source->buffer_size = READER_BUFFER;
            }
            fseek(source->file, source->buffer_start, SEEK_SET);
            source->buffer_size = fread(source->buffer, sizeof(unsigned char), source->buffer_size, source->file);
            source->current = 0;
        }
    }
    value = source->buffer[source->buffer_size - source->current - 1];
    source->current += 1;
    return value;
}
void yield_decoder_bit(struct decode_source * source, unsigned char * value){
    if(source->position == 0){
        if((source->current + 1) >= (source->buffer_size)){
            if(source->stop == 1){
                return 3;
            } else {
                if(source->buffer_start < (READER_BUFFER + source->content_start)){
                    source->buffer_size = source->buffer_start - source->content_start;
                    source->buffer_start = source->content_start;
                    source->stop = 1;
                } else {
                    source->buffer_start = source->buffer_start - (READER_BUFFER);
                    source->buffer_size = READER_BUFFER;
                }
                fseek(source->file, source->buffer_start, SEEK_SET);
                source->buffer_size = fread(source->buffer, sizeof(unsigned char), source->buffer_size, source->file);
                source->current = 0;
            }
        } else {
            source->current += 1;
        }
        source->current_byte = source->buffer[source->buffer_size - source->current - 1];
        source->position = 8;
    }
    *value = source->current_byte & 1;
    source->current_byte = source->current_byte >> 1;
    source->position -= 1;
}
