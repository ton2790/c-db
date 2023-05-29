#ifndef INPUTBUFFER_H
#define INPUTBUFFER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct InputBuffer {
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

InputBuffer *new_input_buffer();
void print_prompt();
void read_input(InputBuffer *input_buffer);
void close_input_buffer(InputBuffer *input_buffer);

#endif
