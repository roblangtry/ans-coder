#include "pANS.h"

int pans_encode()
{
    C_block_t * block = malloc(sizeof(C_block_t));
    t_bwriter * writer = malloc(sizeof(t_bwriter));
    C_data_t * data = malloc(sizeof(C_data_t));
    start_bwriter(writer);
    while((block->len = fread(block->content, sizeof(uint32_t), BLOCK_SIZE, stdin)) > 0){
        //binary_encode(MAGIC, MAGIC_LENGTH, writer);
        parralel_encode_block(block, writer, data);
    }
    nio_flush_bits(writer);
    return 0;
}
int pans_decode()
{
    t_breader * reader = malloc(sizeof(t_breader));
    t_iwriter * writer = malloc(sizeof(t_iwriter));
    D_data_t * data = malloc(sizeof(D_data_t));
    uint32_t V;
    io_back backfeed;
    start_breader(reader);
    start_iwriter(writer);
    backfeed.val = 0;
    backfeed.len = 0;
    while(atend(reader)){
        //if(binary_decode(&V,MAGIC_LENGTH - backfeed.len, reader) == 0) break;
        //V = V + (backfeed.val << (MAGIC_LENGTH - backfeed.len));
        //if(V != MAGIC) break;
        if(parralel_decode_block(reader, writer, &backfeed, data) == 0) break;
    }
    nio_flush_ints(writer);
    return 0;
}
void parralel_encode_block(C_block_t * block, t_bwriter * writer, C_data_t * data)
{
    uint32_t * l = data->l;
    uint32_t * b = data->b;
    uint64_t state;
    uint32_t p1,p2;
    uint32_t Is, x, n=0;
    uint32_t * syms = data->syms;
    uint32_t m = block->len;
    uint32_t xmax = 0;
    uint32_t * M = block->content;
    uint32_t i, track=0, add=0,v,j;
    memset(l, 0, SYMBOL_MAP_SIZE * sizeof(uint32_t));
    bit_buffer * buffer = &data->buffer;
    for(i = 0; i < m; i++)
    {
        x = M[i];
        if(x > xmax) xmax = x;
        if(l[x] == 0){
            n++;
        }
        l[x] += 1;
    }
    n = 0;
    for(i=0;i<=xmax;i++){
        if(l[i]){
            syms[n++] = i;
            b[i] = track;
            track += l[i];
        }
    }
    Is = (m<<1) - 1;
    
    state = m;
    start_buffer(buffer);
    for(i=0;i<m;i++)
    {
        v=0;
        j=0;
        while(state > ((l[M[m-i-1]] << 1) -1)){
            v = (v << 1) + (state & 1);
            j++;
            state = state >> 1;
        }
        buffer_bits(v, j, buffer);
        state = m * (state / l[M[m-i-1]]) + b[M[m-i-1]] + (state % l[M[m-i-1]]);
    }
    p1 = (state - m) >> 32;
    p2 = (state - m) % (1 << 31);
    binary_encode(p1, 32, writer);
    binary_encode(p2, 32, writer);
    elias_delta_encode(n, writer);
    elias_delta_encode(buffer->blen + 1, writer);
    elias_delta_encode(buffer->clen + 1, writer);
    add = 0;
    for(i=0;i<n;i++)
    {
        elias_delta_encode(syms[i]-add, writer);
        elias_delta_encode(l[syms[i]], writer);
        add = syms[i];
    }
    for(i=0;i<buffer->blen;i++)
        binary_encode(buffer->buffer[i], 32, writer);
    binary_encode(buffer->current, buffer->clen, writer);

}
uint32_t parralel_decode_block(t_breader * reader, t_iwriter * writer, io_back * backfeed, D_data_t * data)
{
    uint64_t state;
    uint32_t p1,p2;
    uint32_t n,m,i,j,s,bs,ls,add=0, blen, clen, b_int;
    uint32_t * l = data->l;
    uint32_t * b = data->b;
    uint32_t * syms = data->syms;
    uint32_t * out = data->out;
    uint32_t * table = data->table;
    bit_buffer * buffer = &data->buffer;
    start_buffer(buffer);
    binary_decode(&p1, 32, reader);
    binary_decode(&p2, 32, reader);
    state = (p1 << 32) + p2;
    elias_delta_decode(&n, reader);
    elias_delta_decode(&blen, reader);
    blen--;
    elias_delta_decode(&clen, reader);
    clen--;
    bs = 0;
    for(i=0;i<n;i++)
    {
        elias_delta_decode(&syms[i], reader);
        syms[i] += add;
        add = syms[i];
        elias_delta_decode(&l[i], reader);
        b[i] = bs;
        bs += l[i];
        for(j=0;j<l[i];j++)
        {
            out[b[i] + j] = i;
            table[b[i] + j] = l[i] + j;
        }
    }
    m = bs;
    state += m;
    for(i=0;i<blen;i++){
        binary_decode(&b_int, 32, reader);
        buffer_int(b_int, buffer);
    }
    binary_decode(&b_int, clen, reader);
    set_buffer(b_int,clen,buffer);
    s = out[state % m];
    nio_write_int(syms[s], writer);
    state = l[s] * (state / m) + (state % m) - b[s];
    while(state < m)
    {
        state = (state << 1) + get_buffered_bit(buffer);
    }
    for(i=1;i<m;i++)
    {
        nio_write_int(syms[out[state - m]], writer);
        state = table[state - m];
        while(state < m)
        {
            state = (state << 1) + get_buffered_bit(buffer);
        }
    }
    return 1;
}