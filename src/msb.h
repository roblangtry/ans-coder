#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "constants.h"
#include "page.h"

#ifndef MSB_CODE
#define MSB_CODE
uint32_t get_symbol(uint32_t input, coding_signature_t signature);
uint32_t get_usymbol(uint32_t input, coding_signature_t signature);
uint32_t get_msb_symbol(uint32_t symbol, uint32_t binary_length);
uint32_t get_umsb_symbol(uint32_t symbol, uint32_t binary_length);
void stream_msb(uint32_t symbol, uint32_t bits, int_page_t * pages);
uint32_t get_msb_2_symbol(uint32_t symbol, uint32_t binary_length);
uint32_t get_umsb_2_symbol(uint32_t symbol, uint32_t binary_length);
void stream_msb_2(uint32_t symbol, uint32_t bits, bit_page_t * pages);
#endif