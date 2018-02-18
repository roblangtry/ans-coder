#include <stdlib.h>
#include <stdint.h>
#include "bitlevel_io.h"
#include <math.h>
#include "mem_manager.h"
#ifndef ELIAS_CODE
#define ELIAS_CODE
struct bitlevel_object calculate_elias_value(uint64_t value);
void write_elias_value(struct bitlevel_file_pointer * file_pointer, uint64_t value);
uint64_t read_elias_value(struct bitlevel_file_pointer * file_pointer);
#endif