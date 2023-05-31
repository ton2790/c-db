#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "inputbuffer.h"
#include "metacommand.h"
#include "row.h"
#include "table.h"
#include "statement.h"

int main(int argc, char **argv)
{
  Table *table = new_table();
  InputBuffer* input_buffer = new_input_buffer();
  for(;;) {
    print_prompt();
    read_input(input_buffer);

    if (input_buffer->buffer[0] == '.') {
      switch(do_meta_command(input_buffer)) {
      case META_COMMAND_SUCCESS:
	continue;
      case META_COMMAND_UNRECOGNISED_COMMAND:
	printf("Unrecognised command: '%s'.\n", input_buffer->buffer);
	continue;
      }
    }

    Statement statement;
    switch(prepare_statement(input_buffer, &statement)) {
    case PREPARE_SUCCESS:
      break;
    case PREPARE_SYNTAX_ERROR:
      printf("Error: could not parse statement.\n");
      continue;
    case PREPARE_STRING_TOO_LONG:
      printf("Error: string too long for field.\n");
      continue;
    case PREPARE_NEGATIVE_ID:
      printf("Error: negative id.\n");
      continue;
    case PREPARE_UNRECOGNISED_STATEMENT:
      printf("Unrecognised keyword at start of '%s'.\n", input_buffer->buffer);
      continue;
    }

    switch (execute_statement(&statement, table)) {
    case EXECUTE_SUCCESS:
      printf("Executed.\n");
      break;
    case EXECUTE_TABLE_FULL:
      printf("Error: Table full.\n");
      break;
    }
  }
}
