#include "edit.h"
#include "row.h"
#include "logger.h"
#include "zilo.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Insert characters at the current cursor position 
 *        (handles automatic line wrapping and cursor movement).
 *
 * @param c Inserted character.
 */
void editor_insert_char(int c) {
  // Empty file, at this point E.cy = 0, E.numrows = 0
  if (E.cy == E.numrows) {
    // Append an empty line
    editor_append_row("", 0);
  }

  // Insert character
  erow_t *row = &E.row[E.cy];
  editor_row_insert_char(row, E.cx, c);

  // Update
  E.cx ++;
}

/**
 * @brief Insert a new blank line at position `at`.
 *
 * The 'append_row()' is equivalent to the 'insert_row(E.numrows, ...)'.
 *
 * @param at  Insertion position.
 * @param s   The string on the new line.
 * @param len String length.
 */
void editor_insert_row(int at, char *s, size_t len) {
  if (at < 0 || at > E.numrows) return;

  // Expand `row` array
  erow_t *new = realloc(E.row, sizeof(erow_t) * (E.numrows + 1));
  if (!new) LOG_ERROR("realloc", "Failed to expand memory.");
  E.row = new;

  // Move all lines after the 'at' key to the right
  memmove(&E.row[at + 1], &E.row[at], sizeof(erow_t) * (E.numrows - at));

  // Initialize a newline at the `at` position
  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';

  // Update total number of rows
  E.numrows ++;
}

/**
 * @brief Insert a newline character at the current cursor position (Enter key logic).
 */
void editor_insert_newline(void) {
  if (E.cx == 0) {
    // If the cursor is at the beginning of the line, 
    // simply insert a blank line above the current line
    editor_insert_row(E.cy, "", 0);
  } else {
    // Get the current row object
    erow_t *row = &E.row[E.cy];

    // Calculate the length of the next line
    size_t nex_line_len = row->size - E.cx;

    // Insert a new line
    // The data in the next line is from the cursor position to the end of the line
    editor_insert_row(E.cy + 1, row->chars + E.cx, nex_line_len);

    // Re‑acquire the pointer to the current row 
    // (since E.row may have been relocated by realloc)
    row = &E.row[E.cy];

    // It is now safe to truncate the current line
    row->size = E.cx;
    row->chars[row->size] = '\0';
  }

  // Update cursor
  E.cy ++;
  E.cx = 0;
}

/**
 * @brief Delete the character to the left of the current cursor (Backspace logic).
 */
void editor_del_left_char(void) {
  // Get the current row object
  erow_t *row = &E.row[E.cy];

  if (E.cx > 0) {
    E.cx --;
    editor_row_remove_char(row, E.cx);
    return;
  } else if (E.cx == 0 && E.cy > 0) {
    erow_t *target_row = &E.row[E.cy - 1];
    size_t target_row_len = target_row->size;

    // Append the content of row[cy] to the target row
    editor_row_append_string(target_row, row->chars, row->size);

    // Delete row[cy]
    editor_free_row(row); // Only free chars array
    memmove(&E.row[E.cy], &E.row[E.cy + 1], sizeof(erow_t) * (E.numrows - E.cy - 1));

    // Update
    E.cx = target_row_len;
    E.cy --;
    E.numrows --;
  }
}

/**
 * @brief Delete the current cursor position character (Normal mode 'x' logic).
 */
void editor_del_current_char(void) {
  // Get the current row object
  erow_t *row = &E.row[E.cy];

  editor_row_remove_char(row, E.cx);

  if (E.cx == row->size) E.cx --;
}

/**
 * @brief Delete the specify row. (Normal mdoe 'dd' logic).
 *
 * @param at Position of the deleted line.
 */
void editor_del_row(int at) {
  // Cheeck `at` parameter
  if (at < 0 || at >= E.numrows) return;

  // Free characters array
  editor_free_row(&E.row[at]);

  // Move
  memmove(&E.row[at], &E.row[at + 1], sizeof(erow_t) * (E.numrows - at - 1));

  // Update
  E.numrows --;

  // Correct cursor
  if (at == E.numrows) { // The last line
    E.cy --;
  }

  // Get new line row object
  if (E.cy == 0) {  // After deleting the file, it is empty
    E.cx = 0;
    return;
  }

  erow_t *row = &E.row[E.cy];
  if (E.cx > row->size) {
    E.cx = row->size;
    return;
  }
}
