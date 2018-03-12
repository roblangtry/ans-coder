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
#include "writer.h"
#include "prelude_code.h"
#ifndef ENCODE_CODE
#define ENCODE_CODE
int encode(char *input_filename, char *output_filename, unsigned char method_flag, unsigned char verbose_flag);
int encode_file(FILE * input_file, FILE * output_file, coding_signature_t signature);
#endif