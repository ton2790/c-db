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

void internal_node_split_and_insert(Table *table,
				    uint32_t parent_page_num,
				    uint32_t child_page_num);

void internal_node_insert(Table *table,
			  uint32_t parent_page_num,
			  uint32_t child_page_num)
{
  Pager *pager = table->pager;
  void *parent = get_page(pager, parent_page_num);
  void *child = get_page(pager, child_page_num);
  uint32_t child_max_key = get_node_max_key(pager, child);
  uint32_t index = internal_node_find_child(parent, child_max_key);
  uint32_t original_num_keys = *internal_node_num_keys(parent);
  if (original_num_keys >= INTERNAL_NODE_MAX_KEYS) {
    internal_node_split_and_insert(table, parent_page_num, child_page_num);
    return;
  }
  uint32_t right_child_page_num = *internal_node_right_child(parent);
  if (right_child_page_num == INVALID_PAGE_NUM) {
    *internal_node_right_child(parent) = child_page_num;
    return;
  }
  void *right_child = get_page(pager, right_child_page_num);
  *internal_node_num_keys(parent) = original_num_keys + 1;
  if (child_max_key > get_node_max_key(pager, right_child)) {
    *internal_node_child(parent, original_num_keys) = right_child_page_num;
    *internal_node_key(parent, original_num_keys) =
      get_node_max_key(pager, right_child);
    *internal_node_right_child(parent) = child_page_num;
  } else {
    for (uint32_t i = original_num_keys; i > index; i--) {
      void *destination = internal_node_cell(parent,i);
      void *source = internal_node_cell(parent, i-1);
      memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
    }
    *internal_node_child(parent, index) = child_page_num;
    *internal_node_key(parent, index) = child_max_key;
  }
}

void internal_node_split_and_insert(Table *table,
				    uint32_t parent_page_num,
				    uint32_t child_page_num)
{
  Pager *pager = table->pager;
  uint32_t old_page_num = parent_page_num;
  void *old_node = get_page(pager, old_page_num);
  uint32_t old_max = get_node_max_key(pager, old_node);
  void *child = get_page(pager, child_page_num);
  uint32_t child_max = get_node_max_key(pager, child);
  uint32_t new_page_num = get_unused_page_num(pager);
  uint32_t splitting_root = is_node_root(old_node);
  void *parent;
  void *new_node;
  if (splitting_root) {
    create_new_root(table, new_page_num);
    parent = get_page(pager, table->root_page_num);
    old_page_num = *internal_node_child(parent, 0);
    old_node = get_page(pager, old_page_num);
  } else {
    parent = get_page(pager, *node_parent(old_node));
    new_node = get_page(pager, new_page_num);
    initialize_internal_node(new_node);
  }
  uint32_t *old_num_keys = internal_node_num_keys(old_node);
  uint32_t cur_page_num = *internal_node_right_child(old_node);
  void *cur = get_page(pager, cur_page_num);
  internal_node_insert(table, new_page_num, cur_page_num);
  *node_parent(cur) = new_page_num;
  *internal_node_right_child(old_node) = INVALID_PAGE_NUM;
  for (int i = INTERNAL_NODE_MAX_KEYS - 1; i > INTERNAL_NODE_MAX_KEYS / 2; i--) {
    cur_page_num = *internal_node_child(old_node, i);
    cur = get_page(pager, cur_page_num);
    internal_node_insert(table,  new_page_num, cur_page_num);
    *node_parent(cur) = new_page_num;
    (*old_num_keys)--;
  }
  *internal_node_right_child(old_node)
    = *internal_node_child(old_node, *old_num_keys - 1);
  (*old_num_keys)--;
  uint32_t max_after_split = get_node_max_key(pager, old_node);
  uint32_t destination_page_num
    = child_max < max_after_split ? old_page_num : new_page_num;
  internal_node_insert(table, destination_page_num, child_page_num);
  update_internal_node_key(parent, old_max, get_node_max_key(pager, old_node));
  if (!splitting_root) {
    internal_node_insert(table, *node_parent(old_node), new_page_num);
    *node_parent(new_node) = *node_parent(old_node);
  }
}

void leaf_node_split_and_insert(Cursor *cursor, uint32_t key, Row *value)
{
  Pager *pager = cursor->table->pager;
  void *old_node = get_page(pager, cursor->page_num);
  uint32_t old_max = get_node_max_key(pager, old_node);
  uint32_t new_page_num = get_unused_page_num(pager);
  void *new_node = get_page(pager, new_page_num);
  initialize_leaf_node(new_node);
  *node_parent(new_node) = *node_parent(old_node);
  *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
  *leaf_node_next_leaf(old_node) = new_page_num;
  for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
    void *destination_node;
    if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
      destination_node = new_node;
    } else {
      destination_node = old_node;
    }
    uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
    void *destination = leaf_node_cell(destination_node, index_within_node);
    if (i == cursor->cell_num) {
      serialize_row(value, leaf_node_value(destination_node, index_within_node));
      *leaf_node_key(destination_node, index_within_node) = key;
    } else if (i > cursor->cell_num) {
      memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
    } else {
      memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
    }
  }
  *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
  *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;
  if (is_node_root(old_node)) {
    create_new_root(cursor->table, new_page_num);
  } else {
    uint32_t parent_page_num = *node_parent(old_node);
    uint32_t new_max = get_node_max_key(pager, old_node);
    void *parent = get_page(pager, parent_page_num);
    update_internal_node_key(parent, old_max, new_max);
    internal_node_insert(cursor->table, parent_page_num, new_page_num);
    return;
  }
}

void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value)
{
  void *node = get_page(cursor->table->pager, cursor->page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells >= LEAF_NODE_MAX_CELLS) {
    leaf_node_split_and_insert(cursor, key, value);
    return;
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


