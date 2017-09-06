#include "pANS.h"

int pans_encode()
{
    C_block_t * block = malloc(sizeof(C_block_t));
    t_bwriter * writer = malloc(sizeof(t_bwriter));
    start_bwriter(writer);
    while((block->len = fread(block->content, sizeof(uint32_t), BLOCK_SIZE, stdin)) > 0){
        binary_encode(MAGIC, MAGIC_LENGTH, writer);
        parralel_encode_block(block, writer);
    }
    nio_flush_bits(writer);
    return 0;
}
int pans_decode()
{
    t_breader * reader = malloc(sizeof(t_breader));
    t_iwriter * writer = malloc(sizeof(t_iwriter));
    uint32_t V;
    io_back backfeed;
    start_breader(reader);
    start_iwriter(writer);
    backfeed.val = 0;
    backfeed.len = 0;
    while(atend(reader)){
        if(binary_decode(&V,MAGIC_LENGTH - backfeed.len, reader) == 0) break;
        V = V + (backfeed.val << (MAGIC_LENGTH - backfeed.len));
        if(V != MAGIC) break;
        if(parralel_decode_block(reader, writer, &backfeed) == 0) break;
    }
    nio_flush_ints(writer);
    return 0;
}
void parralel_encode_block(C_block_t * block, t_bwriter * writer)
{
    uint32_t * l = calloc(SYMBOL_MAP_SIZE, sizeof(uint32_t));
    uint32_t * b;
    uint64_t state;
    uint32_t p1,p2;
    uint32_t Is, x, n=0;
    uint32_t * syms;
    uint32_t m = block->len;
    uint32_t xmax = 0;
    uint32_t * M = block->content;
    uint32_t i, track=0, add=0;
    bit_buffer * buffer = malloc(sizeof(bit_buffer));
    for(i = 0; i < m; i++)
    {
        x = M[i];
        if(x > xmax) xmax = x;
        if(l[x] == 0){
            n++;
        }
        l[x] += 1;
    }
    b = malloc(xmax * sizeof(uint32_t));
    syms = malloc(n * sizeof(uint32_t));
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
        while(state > ((l[M[m-i-1]] << 1) -1)){
            buffer_bit(state & 1, buffer);
            //fprintf(stderr, "OUT %d\n", state & 1);
            state = state >> 1;
        }
        //fprintf(stderr, "S (%d)\n", state);
        state = m * (state / l[M[m-i-1]]) + b[M[m-i-1]] + (state % l[M[m-i-1]]);
    }
    //sleep(10);
    //fprintf(stderr, "m = %d\n", m);
    elias_delta_encode(m, writer);
    //fprintf(stderr, "state - m = %u\n", state - m);
    p1 = (state - m) >> 32;
    p2 = (state - m) % (1 << 31);
    binary_encode(p1, 32, writer);
    binary_encode(p2, 32, writer);
    //fprintf(stderr, "n = %d\n", n);
    elias_delta_encode(n, writer);
    //fprintf(stderr, "buffer->blen + 1 = %d\n", buffer->blen + 1);
    elias_delta_encode(buffer->blen + 1, writer);
    //fprintf(stderr, "buffer->clen + 1 = %d\n", buffer->clen + 1);
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
uint32_t parralel_decode_block(t_breader * reader, t_iwriter * writer, io_back * backfeed)
{
    uint64_t state;
    uint32_t p1,p2;
    uint32_t n,m,i,j,s,bs,ls,add=0, blen, clen, b_int;
    uint32_t * l;
    uint32_t * b;
    uint32_t * syms;
    uint32_t * out;
    uint32_t * table;
    bit_buffer * buffer = malloc(sizeof(bit_buffer));

    start_buffer(buffer);
    elias_delta_decode(&m, reader);
    //fprintf(stderr, "m = %d\n", m);
    binary_decode(&p1, 32, reader);
    binary_decode(&p2, 32, reader);
    //fprintf(stderr, "state - m = %d\n", state);
    state = (p1 << 32) + p2;
    state += m;
    elias_delta_decode(&n, reader);
    //fprintf(stderr, "n = %d\n", n);
    elias_delta_decode(&blen, reader);
    //fprintf(stderr, "blen = %d\n", blen);
    blen--;
    elias_delta_decode(&clen, reader);
    //fprintf(stderr, "clen = %d\n", clen);
    clen--;
    syms = malloc(n * sizeof(uint32_t));
    l = malloc(n * sizeof(uint32_t));
    b = malloc(n * sizeof(uint32_t));

    //fprintf(stderr, "Allocated 1\n");
    bs = 0;
    for(i=0;i<n;i++)
    {
        elias_delta_decode(&syms[i], reader);
        syms[i] += add;
        add = syms[i];
        elias_delta_decode(&l[i], reader);
        b[i] = bs;
        bs += l[i];
    }
    //fprintf(stderr, "Filled\n");
    table = malloc(sizeof(uint32_t) * m);
    out = malloc(sizeof(uint32_t) * m);
    //fprintf(stderr, "Allocated 2\n");
    for(i=0;i<blen;i++){
        binary_decode(&b_int, 32, reader);
        buffer_int(b_int, buffer);
    }
    //fprintf(stderr, "Filled\n");
    binary_decode(&b_int, clen, reader);
    set_buffer(b_int,clen,buffer);
    //fprintf(stderr, "Fillet\n");
    j = 0;
    s = syms[j];
    ls = l[j];
    add = ls;
    bs = b[j];
    //fprintf(stderr, "Prepared\n");
    #pragma omp parallel num_threads (THREADS) private(i,j)
    {
        for(i=(omp_get_thread_num() * n);i<MIN(((omp_get_thread_num()+1) * n),n);i++)
        {
            for(j=0;j<l[i];j++)
            {
                out[b[i] + j] = i;
                table[b[i] + j] = l[i] + j;
            }
        }
    }
    //fprintf(stderr, "Buffer initial state %u(%u)[%u]\n", buffer->current, buffer->clen, buffer->blen);
    //fprintf(stderr, "Tabled\n");
    s = out[state % m];
    nio_write_int(syms[s], writer);
    state = l[s] * (state / m) + (state % m) - b[s];
    while(state < m)
    {
        state = (state << 1) + get_buffered_bit(buffer);
    }
    //fprintf(stderr, "first symbol decoded %u (%lu)\n", s, state);
    for(i=1;i<m;i++)
    {
        //fprintf(stderr, "(%lu)\n", state);
        nio_write_int(syms[out[state - m]], writer);
        state = table[state - m];
        while(state < m)
        {
            state = (state << 1) + get_buffered_bit(buffer);
        }
    }
    //fprintf(stderr, "Buffer final state %u(%u)[%u]\n", buffer->read, buffer->clen, buffer->blen);
    //fprintf(stderr, "Decoded %lu\n", state);
    return 1;
}