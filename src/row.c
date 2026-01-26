#include "row.h"
#include "zilo.h"
#include "terminal.h"
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
  if (!new) die("realloc");
  E.row = new;
   
  // Copy string to `chars`
  E.row[E.numrows].chars = malloc(len + 1);
  memcpy(E.row[E.numrows].chars, s, len);
  E.row[E.numrows].chars[len] = '\0';

  // Update
  E.row[E.numrows].size = len;
  E.numrows ++;
}
