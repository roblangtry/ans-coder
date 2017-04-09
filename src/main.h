#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "decode.h"
#include "encode.h"
#define BUFFER_SIZE 4096
void verbose_message(char * message, unsigned char verbose_flag);