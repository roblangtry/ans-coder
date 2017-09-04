#include "nio.h"
const uint32_t BIT_VALUES[] = {
    0,
    1,
    2,
    4,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    2048,
    4096,
    8192,
    16384,
    32768,
    65536,
    131072,
    262144,
    524288,
    1048576,
    2097152,
    4194304,
    8388608,
    16777216,
    33554432,
    67108864,
    134217728,
    268435456,
    536870912,
    1073741824,
    2147483648
};
void start_breader(t_breader * reader){
    reader->length = 0;
    reader->current = 0;
    reader->head = 0;
    reader->buffer_head = 0;
    reader->end = 0;
}
uint32_t nio_get_bit(t_breader * reader, uint32_t * value){
    return nio_get_bits(reader, value, 1);
}
uint32_t nio_get_bits(t_breader * reader, uint32_t * value, uint32_t len)
{
    uint32_t lint, rint;
    if(len > reader->head){
        /* pull from buffer */
        if(reader->buffer_head == reader->length)
        {
            if(reader->end) return 0;
            reader->length = fread(reader->buffer, sizeof(uint32_t), BUFFER_SZ, stdin);
            if(reader->length != BUFFER_SZ) reader->end = 1;
            reader->buffer_head = 0;
        }
        if(reader->buffer_head == reader->length) return 0;
        else
        {
            lint = reader->current << (len - reader->head);
            rint = reader->buffer[reader->buffer_head] >> (32 - (len - reader->head));
            reader->head = 32 - (len - reader->head);
            reader->current = reader->buffer[reader->buffer_head++] % (1 << reader->head);
            *value = lint + rint;
        }
    }
    else
    {
        *value = reader->current >> (reader->head - len);
        reader->current = reader->current - (*value << (reader->head - len));
        reader->head = reader->head - len;
    }
    return 1;
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
void nio_write_bit(uint32_t b, t_bwriter * writer){
    nio_write_bits(b, 1, writer);
}
void nio_write_bits(uint32_t val, uint32_t len, t_bwriter * writer){
    uint32_t bits_from_lint;
    uint32_t bits_from_rint;
    uint32_t lint;
    uint32_t rint;
    if(writer->head + len >= 32)
    {
        bits_from_lint = writer->head;
        bits_from_rint = 32 - bits_from_lint;
        lint = writer->current << bits_from_rint;
        rint = val >> (len - bits_from_rint);
        writer->buffer[writer->buffer_head++] = lint + rint;
        writer->current = val - (rint << (len - bits_from_rint));
        writer->head = len - bits_from_rint;
        if(writer->buffer_head == BUFFER_SZ)
        {
            fwrite(writer->buffer, sizeof(uint32_t), writer->buffer_head, stdout);
            writer->buffer_head = 0;
        }
    }
    else
    {
        writer->current = (writer->current << len) + val;
        writer->head += len;
    }
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
