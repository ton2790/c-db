#include "metacommand.h"

MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table)
{
  if (!strcmp(input_buffer->buffer, ".exit")) {
    db_close(table);
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNISED_COMMAND;
  }
}
