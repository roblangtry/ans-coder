#include <stdio.h>
#include <stdlib.h>
#include "rANS.h"
#include "bANS.h"
#include "vANS.h"
#include "preprocessing.h"
#include "prelude_code.h"
#include "constants.h"
#include "mem_manager.h"
#ifndef DECODE_CODE
#define DECODE_CODE
int decode(char * input_filename, char * output_filename, unsigned char verbose_flag);
#endif