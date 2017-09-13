#include "nio.h"
void start_breader(t_breader * reader){
    reader->length = 0;
    reader->current = 0;
    reader->head = 0;
    reader->buffer_head = 0;
    reader->end = 0;
}

void io_backfeed(t_breader * reader, uint32_t buffer, uint32_t len)
{
    uint64_t value = buffer << reader->head;
    reader->current += value;
    reader->length += len;
}
void start_bwriter(t_bwriter * writer){
    writer->current = 0;
    writer->head = 0;
    writer->buffer_head = 0;
}

void nio_flush_bits(t_bwriter * writer)
{
    if(writer->head > 0)
        writer->buffer[writer->buffer_head++] = writer->current << (32 - writer->head);
    fwrite(writer->buffer, sizeof(uint32_t), writer->buffer_head, stdout);
    writer->buffer_head = 0;
    writer->current = 0;
    writer->head = 0;
}
size_t get_file_size(FILE * stream){
    size_t off, sz;
    off = ftell(stream);
    fseek(stream, 0, SEEK_END);
    sz = ftell(stream);
    fseek(stream, off, SEEK_SET);
    if(off == -1 || sz == -1){
        fprintf(stderr, "File Size cannot be determined\n");
        exit(EXIT_FAILURE);
    }
    return sz;
}
int
atend(t_breader * reader)
{
    if(reader->end == 1 && reader->buffer_head == reader->length) return 0;
    return 1;
}
void
start_iwriter(t_iwriter * writer)
{
    writer->head = 0;
}
void
nio_write_int(uint32_t i, t_iwriter * writer)
{
    writer->buffer[writer->head++] = i;
    if(writer->head == BUFFER_SZ) {
        fwrite(writer->buffer, sizeof(uint32_t), writer->head, stdout);
        writer->head = 0;
    }
}
void
nio_flush_ints(t_iwriter * writer)
{
    if(writer->head) {
        fwrite(writer->buffer, sizeof(uint32_t), writer->head, stdout);
        writer->head = 0;
    }
}
