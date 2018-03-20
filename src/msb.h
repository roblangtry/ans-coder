#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "page.h"

#ifndef MSB_CODE
#define MSB_CODE
uint32_t get_msb_symbol(uint32_t symbol, uint32_t binary_length);
void stream_msb(uint32_t symbol, uint32_t bits, int_page_t * pages);
#endif