#include "bitlevel_io.h"
struct bitlevel_file_pointer * get_bitlevel_file_pointer(FILE * file_pointer){
    // Make a bitlevel_file_pointer from a file pointer
    struct bitlevel_file_pointer * bitlevel_file_pointer;
    // Assign the memory
    bitlevel_file_pointer = (struct bitlevel_file_pointer *)malloc(sizeof(struct bitlevel_file_pointer));
    // Set initial values and file pointer
    bitlevel_file_pointer->file_pointer = file_pointer;
    bitlevel_file_pointer->buffer = 0;
    bitlevel_file_pointer->byte_buffer = malloc(sizeof(unsigned char) * BUFFER_SIZE);
    bitlevel_file_pointer->current_byte = 0;
    bitlevel_file_pointer->bytes_in_buffer = 0;
    bitlevel_file_pointer->current_length = 0;
    bitlevel_file_pointer->use_buffer = 1;
    // Return the pointer
    return bitlevel_file_pointer;
}
struct bitlevel_file_pointer * get_unbuffered_bitlevel_file_pointer(FILE * file_pointer){
    // Make a bitlevel_file_pointer from a file pointer
    struct bitlevel_file_pointer * bitlevel_file_pointer;
    // Assign the memory
    bitlevel_file_pointer = (struct bitlevel_file_pointer *)malloc(sizeof(struct bitlevel_file_pointer));
    // Set initial values and file pointer
    bitlevel_file_pointer->file_pointer = file_pointer;
    bitlevel_file_pointer->buffer = 0;
    bitlevel_file_pointer->current_byte = 0;
    bitlevel_file_pointer->bytes_in_buffer = 0;
    bitlevel_file_pointer->current_length = 0;
    bitlevel_file_pointer->use_buffer = 0;
    // Return the pointer
    return bitlevel_file_pointer;
}

void buffer_write(struct bitlevel_file_pointer * bitlevel_file_pointer, unsigned char byte){
    bitlevel_file_pointer->byte_buffer[bitlevel_file_pointer->bytes_in_buffer] = byte;
    bitlevel_file_pointer->bytes_in_buffer += 1;
    if(bitlevel_file_pointer->bytes_in_buffer == BUFFER_SIZE){
        fwrite(bitlevel_file_pointer->byte_buffer, sizeof(unsigned char), BUFFER_SIZE, bitlevel_file_pointer->file_pointer);
        bitlevel_file_pointer->bytes_in_buffer = 0;
    }
}
unsigned char buffer_read(struct bitlevel_file_pointer * bitlevel_file_pointer){
    unsigned char byte;
    if(bitlevel_file_pointer->use_buffer == 1){
        if(bitlevel_file_pointer->current_byte >= bitlevel_file_pointer->bytes_in_buffer){
            bitlevel_file_pointer->current_byte = 0;
            bitlevel_file_pointer->bytes_in_buffer = fread(bitlevel_file_pointer->byte_buffer, sizeof(unsigned char), BUFFER_SIZE, bitlevel_file_pointer->file_pointer);
        }
        byte = bitlevel_file_pointer->byte_buffer[bitlevel_file_pointer->current_byte];
        bitlevel_file_pointer->current_byte += 1;
    } else{
        fread(&byte, sizeof(unsigned char), 1, bitlevel_file_pointer->file_pointer);
    }
    return byte;
}
size_t buffer_flush(struct bitlevel_file_pointer * bitlevel_file_pointer){
    size_t n;
    n = fwrite(bitlevel_file_pointer->byte_buffer, sizeof(unsigned char), bitlevel_file_pointer->bytes_in_buffer, bitlevel_file_pointer->file_pointer);
    bitlevel_file_pointer->bytes_in_buffer = 0;
    return n;
}
size_t bitlevel_write(struct bitlevel_file_pointer * bitlevel_file_pointer, struct bitlevel_object write_object){
    // Write out bitlevel info
    uint64_t bits_from_lbyte;
    uint64_t bits_from_rbyte;
    size_t no_bytes_written;
    unsigned char byte;
    uint64_t lbyte, rbyte;
    // If there is 8 or more bits then we can write out a byte
    if ((bitlevel_file_pointer->current_length + write_object.length) >= 8){
        // How many bits will be in the left byte component (from the buffer)
        bits_from_lbyte = bitlevel_file_pointer->current_length;
        // How many bits will be in the right byte component (from the write object)
        bits_from_rbyte = 8 - bits_from_lbyte;
        // Get the (left) byte component from the buffer
        lbyte = ((bitlevel_file_pointer->buffer) << bits_from_rbyte);
        // Get the (right) byte component from the write object
        rbyte = (unsigned char)(write_object.value >> (write_object.length - bits_from_rbyte));
        // Combine the left and right components uint64_to a byte
        byte = (unsigned char)(lbyte + rbyte);
        // Amend the write object values to reflect what hasnt been written yet
        write_object.value = write_object.value - (rbyte << (write_object.length - bits_from_rbyte));
        write_object.length = write_object.length - bits_from_rbyte;
        // Write out the calculated byte
        buffer_write(bitlevel_file_pointer, byte);
        no_bytes_written = 1;
        // While there is still enough information in the write object to write a byte
        while(write_object.length >= 8){
            no_bytes_written = no_bytes_written + 1;
            // Pull 8 more bits from the write object uint64_to a byte
            byte = write_object.value >> (write_object.length - 8);
            // Write out that byte
            buffer_write(bitlevel_file_pointer, byte);
            // Recalculate the write object
            write_object.value = write_object.value - (byte << (write_object.length - 8));
            write_object.length = write_object.length - 8;
        }
        // Whatever is left after writing out as many bytes as possible is written to the buffer
        bitlevel_file_pointer->buffer = write_object.value;
        bitlevel_file_pointer->current_length = write_object.length;
    } else{
        // If there wasn't enough info for a new byte append whatever info there was to the buffer
        no_bytes_written = 0;
        bitlevel_file_pointer->buffer = (bitlevel_file_pointer->buffer << write_object.length) + write_object.value;
        bitlevel_file_pointer->current_length = bitlevel_file_pointer->current_length + write_object.length;
    }
    return no_bytes_written;
}
struct bitlevel_object bitlevel_read(struct bitlevel_file_pointer * bitlevel_file_pointer, uint64_t length){
    // Read in bitlevel info from a file
    struct bitlevel_object bitlevel_object;
    unsigned char byte;
    // Assume we are reading in as much info as was specified in the read call
    bitlevel_object.length = length;
    // While their is less info in the buffer than requested pull in more info
    while(length > bitlevel_file_pointer->current_length){
        // Read in a byte
        byte = buffer_read(bitlevel_file_pointer);
        // Append the byte to the buffer and add 8 to the current length
        bitlevel_file_pointer->buffer = (bitlevel_file_pointer->buffer << 8) + (uint64_t)byte;
        bitlevel_file_pointer->current_length = bitlevel_file_pointer->current_length + 8;
    }
    // Take the amount of requested info from the buffer and put it in the write object
    // amending the buffer values to reflect its new state
    bitlevel_object.value = bitlevel_file_pointer->buffer >> (bitlevel_file_pointer->current_length - length);
    bitlevel_file_pointer->buffer = bitlevel_file_pointer->buffer - (bitlevel_object.value << (bitlevel_file_pointer->current_length - length));
    bitlevel_file_pointer->current_length = bitlevel_file_pointer->current_length - length;
    return bitlevel_object;
}
size_t bitlevel_flush(struct bitlevel_file_pointer * bitlevel_file_pointer){
        // Flush any leftover data to disk to make sure its written
        unsigned char byte;
        size_t no_bytes_written;
        no_bytes_written = 0;
        // While there is enough info in the buffer to write a full byte
        while(bitlevel_file_pointer->current_length >= 8){
            no_bytes_written = no_bytes_written + 1;
            // Pull the byte info from the buffer
            byte = bitlevel_file_pointer->buffer >> (bitlevel_file_pointer->current_length - 8);
            // Write out the byte
            buffer_write(bitlevel_file_pointer, byte);
            // Amend the buffer values to reflect there new values
            bitlevel_file_pointer->current_length = bitlevel_file_pointer->current_length - 8;
            bitlevel_file_pointer->buffer = bitlevel_file_pointer->buffer - (byte << (bitlevel_file_pointer->current_length - 8));
        }
        // If there is still info left to write then write
        if (bitlevel_file_pointer->current_length > 0){
            // Move the values over to ensure they will be read at the other side
            byte = (unsigned char)(bitlevel_file_pointer->buffer << (8 - bitlevel_file_pointer->current_length));
            // Write the byte
            buffer_write(bitlevel_file_pointer, byte);
            no_bytes_written = no_bytes_written + 1;
            // Zero out the buffer
            bitlevel_file_pointer->current_length = 0;
            bitlevel_file_pointer->buffer = 0;
        }
        // Flush to ensure the file system will write the info not just hold it in memory
        buffer_flush(bitlevel_file_pointer);
        fflush(bitlevel_file_pointer->file_pointer);
        return no_bytes_written;
}