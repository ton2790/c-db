#include "table.h"

void *row_slot(Table *table, uint32_t row_num)
{
  uint32_t page_num = row_num / ROWS_PER_PAGE;
  void *page = get_page(table->pager, page_num);
  uint32_t row_offset = row_num % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;
  return page + byte_offset;
}

Table *db_open(const char *filename)
{
  Pager *pager = pager_open(filename);
  uint32_t num_rows = pager->file_length / ROW_SIZE;
  Table *table = (Table *)malloc(sizeof(Table));
  table->pager = pager;
  table->num_rows = num_rows;
  return table;
}

void db_close(Table *table)
{
  Pager *pager = table->pager;
  uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;
  for (uint32_t i = 0; i < num_full_pages; i++) {
    if (pager->pages[i] == NULL) {
      continue;
    }
    pager_flush(pager, i, PAGE_SIZE);
    free(pager->pages[i]);
    pager->pages[i] = NULL;
  }
  uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
  if (num_additional_rows > 0) {
    uint32_t page_num = num_full_pages;
    if (pager->pages[page_num] != NULL) {
      pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
      free(pager->pages[page_num]);
      pager->pages[page_num] = NULL;
    }
  }
  int result = close(pager->file_descriptor);
  if (result == -1) {
    printf("Error closing db file");
    exit(EXIT_FAILURE);
  }
  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    void *page = pager->pages[i];
    if (page) {
      free(page);
      pager->pages[i] = NULL;
    }
  }
  free(pager);
  free(table);
}
