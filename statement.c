#include "statement.h"

PrepareResult prepare_insert(InputBuffer *input_buffer, Statement *statement)
{
  statement->type = STATEMENT_INSERT;
  const char *delim = " ";
  char *keyword = strtok(input_buffer->buffer, delim);
  char *id_str = strtok(NULL, delim);
  char *username = strtok(NULL, delim);
  char *email = strtok(NULL, delim);
  if (id_str == NULL || username == NULL || email == NULL) {
    return PREPARE_SYNTAX_ERROR;
  }
  int id = atoi(id_str);
  if (id < 0) {
    return PREPARE_NEGATIVE_ID;
  }
  if (strlen(username) > COLUMN_USERNAME_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
  if (strlen(email) > COLUMN_EMAIL_SIZE) {
    return PREPARE_STRING_TOO_LONG;
  }
  statement->row_to_insert.id = id;
  strcpy(statement->row_to_insert.username, username);
  strcpy(statement->row_to_insert.email, email);
  return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
  if (!strncmp(input_buffer->buffer, "insert", 6)) {
    return prepare_insert(input_buffer, statement);
  }
  if (!strncmp(input_buffer->buffer, "select", 6)) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }
  return PREPARE_UNRECOGNISED_STATEMENT;
}

void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value)
{
  void *node = get_page(cursor->table->pager, cursor->page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells >= LEAF_NODE_MAX_CELLS) {
    printf("Leaf node splitting unimplemented.\n");
    exit(EXIT_FAILURE);
  }
  if (cursor->cell_num < num_cells) {
    for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
      memcpy(leaf_node_cell(node,i),leaf_node_cell(node,i-1), LEAF_NODE_CELL_SIZE);
    }
  }
  *(leaf_node_num_cells(node)) += 1;
  *(leaf_node_key(node, cursor->cell_num)) = key;
  serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

ExecuteResult execute_insert(Statement *statement, Table *table)
{
  void *node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells >= LEAF_NODE_MAX_CELLS) {
    return EXECUTE_TABLE_FULL;
  }
  Row * row_to_insert = &(statement->row_to_insert);
  uint32_t key_to_insert = row_to_insert->id;
  Cursor *cursor = table_find(table, key_to_insert);
  if (cursor->cell_num < num_cells) {
    uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
    if (key_to_insert == key_at_index) {
      return EXECUTE_DUPLICATE_KEY;
    }
  }
  leaf_node_insert(cursor, row_to_insert->id, row_to_insert);
  free(cursor);
  return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, Table *table)
{
  Cursor *cursor = table_start(table);
  Row row;
  while (!cursor->end_of_table) {
    deserialize_row(cursor_value(cursor), &row);
    print_row(&row);
    cursor_advance(cursor);
  }
  free(cursor);
  return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement *statement, Table *table)
{
  switch(statement->type) {
  case STATEMENT_INSERT:
    return execute_insert(statement, table);
  case STATEMENT_SELECT:
    return execute_select(statement, table);
  }
}


