#include "preprocessing.h"

//struct header {
//    size_t symbol_size;
//    uint64_t no_symbols;
//    uint64_t no_unique_symbols;
//    uint64_t * symbols;
//    uint64_t * symbol_frequencies;
//}
struct header preprocess(FILE * input_file){
    uint * buffer;
    size_t elements_read;
    size_t i, n;
    struct header header;
    uint64_t index;
    uint64_t symbol;
    header.max_symbol = 0;
    header.coding = 0;
    header.no_symbols = 0;
    header.no_unique_symbols = 0;
    header.hashmap = calloc(MAX_SYMBOL, sizeof(uint64_t));
    //header.hashmap = initialise_hashmap();
    buffer = malloc(sizeof(uint) * BUFFER_SIZE);
    while((elements_read = fread(buffer, sizeof(uint), BUFFER_SIZE, input_file)) != 0){
        i = 0;
        while(i<elements_read){
            symbol = buffer[i];
            header.hashmap[symbol] += 1;
            if(symbol > header.max_symbol){
                header.max_symbol = symbol;
            }
            header.no_symbols += 1;
            i++;
        }
    }
    header.symbols = malloc(sizeof(uint64_t) * header.max_symbol);
    header.symbol_frequencies = malloc(sizeof(uint64_t) * header.max_symbol);
    i = 0;
    n = 0;
    while(i <= header.max_symbol){
        if(header.hashmap[i]>0){
            header.symbols[n] = i;
            header.symbol_frequencies[n] = header.hashmap[i];
            header.hashmap[i] = n;
            n++;
        }
        i++;
    }
    header.no_unique_symbols = n;
    return header;
}
void ** initialise_hashmap(){
    void ** pointer;
    size_t i;
    i = 0;
    pointer = malloc(sizeof(void*) * HASHMAP_SIZE);
    while(i < HASHMAP_SIZE){
        pointer[i++] = NULL;
    }
    return pointer;
}
uint64_t get_symbol_index(uint64_t symbol, struct header * header){
    struct hashmap_node * current;
    struct hashmap_node * last;
    struct hashmap_node * node;
    last = NULL;
    current = header->hashmap[symbol % HASHMAP_SIZE];
    if (current == NULL) {
        node = add_symbol(symbol, header);
        header->hashmap[symbol % HASHMAP_SIZE] = node;
        return node->index;
    } else {
        while(current->next != NULL && current->symbol != symbol){
            last = current;
            current = current->next;
        }
        if(current->symbol == symbol){
            if(last != NULL)
                check_for_rearrangement(current, last, header);
            return current->index;
        } else{
            node = add_symbol(symbol, header);
            current->next = node;
            return node->index;
        }
    }
}
uint64_t safe_get_symbol_index(uint64_t symbol, struct header * header){
    return header->hashmap[symbol];
}

struct hashmap_node * add_symbol(uint64_t symbol, struct header * header){
    struct hashmap_node * node;
    node = malloc(sizeof(struct hashmap_node));
    node->symbol = symbol;
    node->index = header->no_unique_symbols;
    node->next = NULL;
    header->no_unique_symbols += 1;
    header->symbols = realloc(header->symbols, sizeof(uint64_t) * header->no_unique_symbols);
    header->symbol_frequencies = realloc(header->symbol_frequencies, sizeof(uint64_t) * header->no_unique_symbols);
    header->symbols[header->no_unique_symbols-1] = symbol;
    header->symbol_frequencies[header->no_unique_symbols-1] = 0;
    return node;
}
void check_for_rearrangement(struct hashmap_node * current, struct hashmap_node * last, struct header * header){
    uint64_t current_val;
    uint64_t last_val;
    uint64_t store_index, store_symbol;
    current_val = header->symbol_frequencies[current->index];
    last_val = header->symbol_frequencies[last->index];
    if(current_val > last_val){
        store_symbol = last->symbol;
        store_index = last->index;
        last->symbol = current->symbol;
        last->index = current->index;
        current->symbol = store_symbol;
        current->index = store_index;
    }
}
void writeout_header(FILE * output_file, struct header header, unsigned char flag_byte){
    fwrite(&flag_byte, sizeof(unsigned char), 1, output_file);
    fwrite(&(header.no_symbols), sizeof(uint64_t), 1, output_file);
    fwrite(&(header.no_unique_symbols), sizeof(uint64_t), 1, output_file);
    fwrite(header.symbols, sizeof(uint64_t), header.no_unique_symbols, output_file);
    fwrite(header.symbol_frequencies, sizeof(uint64_t), header.no_unique_symbols, output_file);
}
struct header read_header(FILE * input_file, unsigned char * flag_byte){
    struct header header;
    header.coding = 1;
    fread(flag_byte, sizeof(unsigned char), 1, input_file);
    fread(&(header.no_symbols), sizeof(uint64_t), 1, input_file);
    fread(&(header.no_unique_symbols), sizeof(uint64_t), 1, input_file);
    header.symbols = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    header.symbol_frequencies = malloc(sizeof(uint64_t) * header.no_unique_symbols);
    fread(header.symbols, sizeof(uint64_t), header.no_unique_symbols, input_file);
    fread(header.symbol_frequencies, sizeof(uint64_t), header.no_unique_symbols, input_file);
    return header;
}