#include "elias.h"
struct bitlevel_object calculate_elias_value(uint64_t value){
    uint64_t length;
    struct bitlevel_object bl_obj;
    // get the largest length value such that 2^length =< value < 2^(length+1)
    length = (floor(log(value) / log(2)) * 2) + 1;
    // Write the values to the object
    bl_obj.value = value;
    bl_obj.length = length;
    return bl_obj;
}
void write_elias_value(struct bitlevel_file_pointer * file_pointer, uint64_t value){
    struct bitlevel_object write_object;
    uint64_t zero_component_length, value_component;
    // This conditional code is to allow negatives and zero to work with the elias coding
    if(value < 0){
        // Value is negative so set signing bit to 1
        write_object.value = 1;
        write_object.length = 1;
        bitlevel_write(file_pointer, write_object);
        write_object = calculate_elias_value(abs(value));
    } else {
        // Value is positive so set signing bit 0
        write_object.value = 0;
        write_object.length = 1;
        bitlevel_write(file_pointer, write_object);
        // add 1 to the value so that 0 is never encountered
        write_object = calculate_elias_value(value + 1);
    }
    // Elias has this lovely habit of being larger than a 32 bit uint64_t in large files so 
    // split out the zero component and write that then write the value component
    zero_component_length = (write_object.length - 1) / 2;
    value_component = write_object.value;
    // Write the zero component
    write_object.value = 0;
    write_object.length = zero_component_length;
    bitlevel_write(file_pointer, write_object);
    // Write the value component
    write_object.value = value_component;
    write_object.length = zero_component_length + 1;
    bitlevel_write(file_pointer, write_object);
}
uint64_t read_elias_value(struct bitlevel_file_pointer * file_pointer){
    uint64_t no_zeroes;
    uint64_t value;
    uint64_t negative_case;
    struct bitlevel_object read_object;
    // Read the signing bit
    read_object = bitlevel_read(file_pointer, 1);
    negative_case = read_object.value;
    // Read the first bit of the value
    read_object = bitlevel_read(file_pointer, 1);
    no_zeroes = 0;
    // While the input read is a zero
    while(read_object.value == 0){
        no_zeroes++;
        read_object = bitlevel_read(file_pointer, 1);
    }
    // Value should be 1
    value = read_object.value;
    // Read in the rest of the elias value (same amount of
    // bits as the zeroes we have read)
    read_object = bitlevel_read(file_pointer, no_zeroes);
    // Push forward the one we got to allow for inclusion of the rest of the value
    value = value << no_zeroes;
    // Add the rest
    value = value + read_object.value;
    // Check the signing bit and amend the value accordingly
    if(negative_case == 1){
        value = value * -1;
    } else {
        value = value - 1;
    }
    return value;
}