#ifndef ROW_H
#define ROW_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define SIZE_OF_ATTRIBUTE(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

typedef struct Row {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

#define ID_SIZE SIZE_OF_ATTRIBUTE(Row, id)
#define USERNAME_SIZE SIZE_OF_ATTRIBUTE(Row, username)
#define EMAIL_SIZE SIZE_OF_ATTRIBUTE(Row, email)
#define ROW_SIZE (ID_SIZE + USERNAME_SIZE + EMAIL_SIZE)
#define ID_OFFSET 0
#define USERNAME_OFFSET (ID_OFFSET + USERNAME_SIZE)
#define EMAIL_OFFSET (ID_OFFSET + EMAIL_SIZE)


void print_row(Row *row);
void serialize_row(Row *source, void *destination);
void deserialize_row(void *source, Row *destination);

#endif
