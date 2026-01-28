#include "row.h"
#include "logger.h"
#include "zilo.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief  Append the string `s` (length `len`) as a new line to the end of the editor.
 *
 * @param s   Appended string.
 * @param len String length.
 */
void editor_append_row(char *s, size_t len) {
  // Expand memory size
  erow_t *new = realloc(E.row, sizeof(erow_t) * (E.numrows + 1));
  if (!new) LOG_ERROR("realloc", "Failed to expand memory.");
  E.row = new;
   
  // Copy string to `chars`
  E.row[E.numrows].chars = malloc(len + 1);
  memcpy(E.row[E.numrows].chars, s, len);
  E.row[E.numrows].chars[len] = '\0';

  // Update
  E.row[E.numrows].size = len;
  E.numrows ++;
}

/**
 * @brief Release memory for one row (used for deleting a row).
 *
 * @param row Memory needs to be freed for the row object.
 */
void editor_free_row(erow_t *row) {
  if (!row) return;

  free(row->chars);
}

/**
 * @brief Insert the character `c` at position `at` in row.
 *
 * @param row Pointer to row object.
 * @param at  Insert position.
 * @param c   Inserted character.
 */
void editor_row_insert_char(erow_t *row, int at, int c) {
  if (!row) return;
  if (at < 0 || at > row->size) return;

  // Expand memory.
  char *new = realloc(row->chars, row->size + 2); // new character + '\0'
  if (!new) LOG_ERROR("realloc", "Failed to expand memory.");
  row->chars = new;

  // Move the data starting from position `at` to position `at + 1`.
  memmove(row->chars + at + 1, row->chars + at, row->size - at);

  // Update.
  row->chars[at] = c;
  row->size ++;
}

/**
 * @brief Remove the character `c` at position `at` in row.
 *
 * @param row Pointer to row object.
 * @param at  Remove position.
 */
void editor_row_remove_char(erow_t *row, int at) {
  if (!row) return;
  if (at < 0 || at >= row->size) return;

  // Move the data starting from position `at + 1` to position `at`.
  memmove(row->chars + at, row->chars + at + 1, row->size - at - 1);

  // Update.
  row->size --;
}

/**
 * @brief Concatenate the string to the end of row.
 *
 * @param row A row object that requires appending a string.
 * @param s   The string to be appended.
 * @param len String length.
 */
void editor_row_append_string(erow_t *row, char *s, size_t len) {
  if (!row) return;

  // Expand char array memory
  char *new = realloc(row->chars, row->size + len + 1);
  if (!new) LOG_ERROR("realloc", "Failed to expand memory.");
  row->chars = new;

  // Append string to the end of row
  memcpy(row->chars + row->size, s, len);
  row->chars[row->size + len] = '\0';

  // Update
  row->size += len;
}
