#include "metacommand.h"

void print_constants()
{
  printf("ROW_SIZE: %d\n", ROW_SIZE);
  printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
  printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
  printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
  printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
  printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

void print_leaf_node(void *node)
{
  uint32_t num_cells = *leaf_node_num_cells(node);
  printf("leaf (size %d)\n", num_cells);
  for (uint32_t i = 0; i < num_cells; i++) {
    uint32_t key = *(leaf_node_key(node, i));
    printf("  - %d : %d\n", i, key);
  }
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table)
{
  if (!strcmp(input_buffer->buffer, ".exit")) {
    db_close(table);
    exit(EXIT_SUCCESS);
  } else if (!strcmp(input_buffer->buffer, ".constants")) {
    printf("Constants: \n");
    print_constants();
    return META_COMMAND_SUCCESS;
  } else if (!strcmp(input_buffer->buffer, ".btree")) {
    printf("Tree: \n");
    print_leaf_node(get_page(table->pager, 0));
    return META_COMMAND_SUCCESS;
  } else {
    return META_COMMAND_UNRECOGNISED_COMMAND;
  }
}
