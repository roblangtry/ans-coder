#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "reader.h"
#include "writer.h"
#include "prelude_code.h"
#include "prelude.h"
#include "bANS.h"
#ifndef VANS_CODE
#define VANS_CODE
void vANS_encode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions);
void vANS_decode(FILE * input_file, FILE * output_file, struct prelude_functions * my_prelude_functions);
#endif