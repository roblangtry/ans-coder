#include <stdio.h>
#include <stdlib.h>
#include "rANS.h"
#include "bANS.h"
#include "preprocessing.h"
#include "prelude_code.h"
#include "constants.h"
#include "mem_manager.h"
#ifndef ENCODE_CODE
#define ENCODE_CODE
int encode(char *input_filename, char *output_filename, unsigned char method_flag, unsigned char verbose_flag);
#endif