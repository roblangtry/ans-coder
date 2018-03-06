#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "rANS.h"
#include "bANS.h"
#include "preprocessing.h"
#include "prelude_code.h"
#include "constants.h"
#include "mem_manager.h"
#include "file_header.h"
#include "block.h"
#ifndef DECODE_CODE
#define DECODE_CODE
int decode(char * input_filename, char * output_filename, unsigned char verbose_flag);
int decode_file(FILE * input_file, FILE * output_file, coding_signature_t signature);
#endif