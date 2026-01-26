#include "output.h"
#include "terminal.h"
#include "zilo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct abuf {
  char *buf;
  int len;
} abuf_t;

/**
 * @brief Append string s to buffer.
 *
 * @param ab  Buffer.
 * @param s   String.
 * @param len Length of string.
 */
static void ab_append(abuf_t *ab, const char *s, int len) {
  // Request a larger memory block
  char *new = realloc(ab->buf, ab->len + len);
  if (!new) return;

  // Copy the new string `s` to the end of the old data
  memcpy(new + ab->len, s, len);

  // Update structure infomation
  ab->buf = new;
  ab->len += len;
}

/**
 * @brief Release buffer memory.
 *
 * @param ab Buffer.
 */
static void ab_free(abuf_t *ab) {
  if (!ab) return;

  free(ab->buf);
}

/**
 * @brief Implement screen scrolling.
 */
static void editor_scroll(void) {
  // Scroll up: If the cursor is above the viewport
  if (E.cy < E.rowoff) {
    E.rowoff = E.cy;
  }

  // Scroll down: If the cursor moves below the viewport
  if (E.cy >= E.rowoff + E.screenrows) {
    E.rowoff = E.cy - E.screenrows + 1;
  }

  // Scroll left: If the cursor is left the viewport
  if (E.cx < E.coloff) {
    E.coloff = E.cx;
  }

  // Scroll right: If the cursor is right the viewport
  if (E.cx >= E.coloff + E.screencols) {
    E.coloff = E.cx - E.screencols + 1;
  }
}

/**
 * @brief Draw the logic for each row (tilde ~).
 *
 * @param ab Buffer.
 */
static void editor_draw_rows(abuf_t *ab) {
  // Loop
  for (int y = 0; y < E.screenrows; ++ y) {
    int filerow = y + E.rowoff;
    if (filerow >= E.numrows) {
      ab_append(ab, "~", 1);
    } else {
      char *line = E.row[filerow].chars;
      int size = E.row[filerow].size;

      // NOTE: Considering horizontal offset
      // If coloff exceeds the line length, 
      // it means that this line is invisible 
      // in the current viewport and nothing should be drawn.
      int len = 0;
      if (size > E.coloff)
        len = size - E.coloff;
      if (len > E.screencols)
        len = E.screencols;

      if (len > 0)
        ab_append(ab, line + E.coloff, len);
    }
    
    // NOTE: Clear residual characters to the right of the cursor at the end of each line.
    ab_append(ab, ANSI_CLEAR_LINE, strlen(ANSI_CLEAR_LINE));
    if (y < E.screenrows - 1) {
      // If it's not the last line, add a newline character
      ab_append(ab, "\r\n", 2);
    }
  }
}

/**
 * @brief Clears the screen, draws a tilde, moves the cursor, 
 *        and finally refreshes the screen.
 */
void editor_refresh_screen(void) {
  editor_scroll();

  abuf_t ab = {NULL, 0};

  ab_append(&ab, ANSI_CURSOR_HIDE, strlen(ANSI_CURSOR_HIDE));
  // NOTE: Reset the cursor before rendering; otherwise, rendering will start 
  // from the previous cursor position, leaving residual characters on the left side.
  ab_append(&ab, ANSI_CURSOR_HOME, strlen(ANSI_CURSOR_HOME));
  editor_draw_rows(&ab);
  ab_append(&ab, ANSI_CURSOR_SHOW, strlen(ANSI_CURSOR_SHOW));

  char buf[32];
  // NOTE: Considering vertical and horizontal offsets
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.rowoff + 1, E.cx - E.coloff + 1);
  ab_append(&ab, buf, strlen(buf));

  write(STDOUT_FILENO, ab.buf, ab.len);

  ab_free(&ab);
}
