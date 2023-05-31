#ifndef STATEMENT_H
#define STATEMENT_H

#include <stdint.h>
#include "inputbuffer.h"
#include "row.h"
#include "table.h"
#include "cursor.h"

typedef enum PrepareResult {
  PREPARE_SUCCESS,
  PREPARE_SYNTAX_ERROR,
  PREPARE_STRING_TOO_LONG,
  PREPARE_NEGATIVE_ID,
  PREPARE_UNRECOGNISED_STATEMENT
} PrepareResult;

typedef enum StatementType {
  STATEMENT_INSERT,
  STATEMENT_SELECT
} StatementType;

typedef struct Statement {
  StatementType type;
  Row row_to_insert;
} Statement;

typedef enum ExecuteResult {
  EXECUTE_SUCCESS,
  EXECUTE_TABLE_FULL
} ExecuteResult;

PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement);
ExecuteResult execute_insert(Statement *statement, Table *table);
ExecuteResult execute_select(Statement *statement, Table *table);
ExecuteResult execute_statement(Statement *statement, Table *table);

#endif
