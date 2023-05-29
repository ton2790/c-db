#ifndef METACOMMAND_H
#define METACOMMAND_H

#include <string.h>
#include "inputbuffer.h"

typedef enum MetaCommandResult {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNISED_COMMAND
} MetaCommandResult;

MetaCommandResult do_meta_command(InputBuffer *input_buffer);

#endif
