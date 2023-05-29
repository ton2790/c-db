#include "metacommand.h"

MetaCommandResult do_meta_command(InputBuffer *input_buffer)
{
  if (!strcmp(input_buffer->buffer, ".exit")) {
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNISED_COMMAND;
  }
}
