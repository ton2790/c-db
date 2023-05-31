#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>
#include <stdint.h>
#include "row.h"
#include "pager.h"

typedef struct Table {
  Pager *pager;
  uint32_t num_rows;
} Table;

void *row_slot(Table *table, uint32_t row_num);
Table *db_open(const char *filename);
void db_close(Table *table);

#endif
