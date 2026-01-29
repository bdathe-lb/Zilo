#ifndef ZILO_ROW_H
#define ZILO_ROW_H

#include "zilo.h"
#include <stddef.h>

// Append the string `s` (length `len`) as a new line to the end of the editor.
void editor_append_row(char *s, size_t len);

// Insert the character `c` at position `at` in row.
void editor_row_insert_char(erow_t *row, int at, int c);

// Remove the character `c` at position `at` in row.
void editor_row_remove_char(erow_t *row, int at);

// Concatenate the string to the end of row.
void editor_row_append_string(erow_t *row, char *s, size_t len);

// Release memory for one row.
void editor_free_row(erow_t *row);

#endif // !ZILO_ROW_H
