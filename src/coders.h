#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "nio.h"
#include "constants.h"
#ifndef CODERS_CODE
#define CODERS_CODE

void binary_encode(uint32_t value, uint32_t length, t_bwriter * writer);
uint32_t binary_decode(uint32_t * V, uint32_t length, t_breader * reader);
void unary_encode(uint32_t value, t_bwriter * writer);
uint32_t unary_decode(uint32_t * V, t_breader * reader);
void elias_gamma_encode(uint32_t value, t_bwriter * writer);
uint32_t elias_gamma_decode(uint32_t * V, t_breader * reader);
void elias_delta_encode(uint32_t value, t_bwriter * writer);
uint32_t elias_delta_decode(uint32_t * V, t_breader * reader);
uint32_t mylog(uint32_t value);
uint32_t flylog( uint32_t x );
#endif