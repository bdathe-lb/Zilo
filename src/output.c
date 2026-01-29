#define _POSIX_C_SOURCE 200809L

#include "output.h"
#include "logger.h"
#include "terminal.h"
#include "zilo.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

static const char *editor_mode_strings[] = {
  "NORMAL",
  "INSERT",
  "REPLACE"
};

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
 * @brief Set status message.
 *
 * @param fmt String to be formatted.
 */
void editor_set_status_message(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
  va_end(ap);

  // Record the current time and start the countdown.
  E.statusmsg_time = time(NULL);
}

/**
 * @brief Draw a status bar.
 */
static void editor_draw_message_bar(abuf_t *ab) {
  // Clear expired messages
  if (time(NULL) - E.statusmsg_time > 1) return;
  
  // If the message is empty, do not draw
  if (E.statusmsg[0] == '\0') return;

  // Calculation display position: last line
  int msg_row = E.screenrows - 1;

  // Move the cursor to the specified row, column 1
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", msg_row, 0);
  ab_append(ab, buf, strlen(buf));

  // Truncate messages (to prevent them from exceeding screen width)
  int msglen = strlen(E.statusmsg);
  if (msglen > E.screencols) {
    msglen = E.screencols;
    ab_append(ab, E.statusmsg, msglen);
  }

  int len = 0;
  while (msglen < E.screencols) {
    if (E.screencols - len == msglen) {
      ab_append(ab, E.statusmsg, msglen);
      break;
    }

    ab_append(ab, " ", 1);
    len ++;
  }

  // Restore Style
  ab_append(ab, ANSI_RESET, strlen(ANSI_RESET));
}

/**
 * @brief Draw a status bar at the bottom.
 */
static void editor_draw_status_bar(abuf_t *ab) {
  ab_append(ab, ANSI_REVERSE_DISPLAY, strlen(ANSI_REVERSE_DISPLAY));

  // Construct the string on the left (filename, line number)
  char lstatus_buf[80];
  int lstatus_len = snprintf(lstatus_buf, sizeof(lstatus_buf), "%.20s - %d lines",
    E.filename ? E.filename : "[No Name]", E.numrows);

  if ((unsigned int)lstatus_len > sizeof(lstatus_buf) - 1) {
    lstatus_len = sizeof(lstatus_buf) - 1;
  } else if (lstatus_len < 0) {
    LOG_ERROR("snprintf", "Failed to construct string on the left.");
    return;
  }

  // Construct the string on the right (current mode, current time HH:MM:SS)
  char rstatus_buf[80];

  // Get local time 
  time_t now = time(NULL);        // Returns UNIX timestamp
  struct tm tm_now;               
  localtime_r(&now, &tm_now); // Convert `time_t` into a human time structure `struct tm`

  int rstatus_len = snprintf(rstatus_buf, sizeof(rstatus_buf), "%s | %d/%d | %02d:%02d",
    editor_mode_strings[E.mode], 
    E.cy + 1, E.numrows,
    tm_now.tm_hour, tm_now.tm_min);

  if ((unsigned int)rstatus_len > sizeof(rstatus_buf) - 1) {
    rstatus_len = sizeof(rstatus_buf) - 1;
  } else if (rstatus_len < 0) {
    LOG_ERROR("snprintf", "Failed to construct string on the right.");
    return;
  }

  // Calculate the number of spaces to fill
  if (lstatus_len > E.screencols) lstatus_len = E.screencols;
  ab_append(ab, lstatus_buf, lstatus_len);

  // Only begin drawing the right side when there is enough remaining space 
  // to fit the right-side status bar.
  // Otherwise, it will keep filling in spaces (equivalent to automatically 
  // hiding the information on the right side when the window is too narrow).
  while (lstatus_len < E.screencols) {
    if (E.screencols - lstatus_len == rstatus_len) {
      ab_append(ab, rstatus_buf, rstatus_len);
      break;
    } else {
      ab_append(ab, " ", 1);
      lstatus_len ++;
    }
  }

  ab_append(ab, ANSI_RESET, strlen(ANSI_RESET));
}

/**
 * @brief Draw the logic for each row (tilde ~).
 *
 * @param ab Buffer.
 */
static void editor_draw_rows(abuf_t *ab) {
  // Loop
  for (int y = 0; y < E.screenrows - 1; ++ y) {
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

  // 1. Draw text content
  editor_draw_rows(&ab);

  // 2. Draw the status bar at the bottom
  editor_draw_status_bar(&ab);

  // 3. Floating Messages
  editor_draw_message_bar(&ab);

  if (E.mode == MODE_INSERT) {
    ab_append(&ab, ANSI_CURSOR_SHAPE_BAR, strlen(ANSI_CURSOR_SHAPE_BAR));
  } else {
    ab_append(&ab, ANSI_CURSOR_SHAPE_BLOCK, strlen(ANSI_CURSOR_SHAPE_BLOCK));
  }
  ab_append(&ab, ANSI_CURSOR_SHOW, strlen(ANSI_CURSOR_SHOW));

  char buf[32];
  // NOTE: Considering vertical and horizontal offsets
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.rowoff + 1, E.cx - E.coloff + 1);
  ab_append(&ab, buf, strlen(buf));

  write(STDOUT_FILENO, ab.buf, ab.len);

  ab_free(&ab);
}
