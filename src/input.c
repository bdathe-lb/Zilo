#include "input.h"
#include "edit.h"
#include "file.h"
#include "terminal.h"
#include "zilo.h"
#include <stdlib.h>

static void cursor_move(char c) {
  // Get the current row object (to prevent crashes when file is empty)
  // If file is empty:
  //  - E.cy is 0
  //  - E.numrows is 0
  //  - E.row is NULL
  erow_t *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

  switch (c) {
    case 'h':  // Left
      if (E.cx > 0) E.cx --;
      break;
    case 'l':  // Right
      // The cursor only moves if row exists 
      // and cursor is not at the end of the line
      if (row && E.cx < row->size) E.cx ++;
      break;
    case 'k':  // Up
      if (E.cy > 0) E.cy --;
      break;
    case 'j':  // Down
      if (E.cy < E.numrows - 1) E.cy ++;
      break;
  }

  // NOTE: Long-line to short-line transtion
  // If only `E.cy` is updated without checking the new line length, issues may arise.
  // Assume the file content is as follows:
  //  - Line 0: "Hello World" (length 11, `E.cx` at position 10)
  //  - Line 1: "Hi" (length 2)
  // When j (move down) is pressed:
  //  - `E.cy` becomes 1
  //  - `E.cx` remains 10
  // The cursor position is now (1, 10). However, line 1 has only 2 characters!
  // Attempting to render or access `E.row[1].chars[10]` will cause an out-of bounds
  // memory access. Moreover, the cursor would also be rendered at a position where 
  // no text exists.

  // After moving, `cy` may have changed, requiring a new row to be retrieved
  row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
  // int rowlen = row ? row->size - E.coloff - 1 : 0;
  int rowlen = row ? row->size : 0;

  // NOTE: For blank rows, rowlen is 0, and E.cx is -1, 
  // so we need to ensure that blank rows remain in column 0.
  if (E.cx > rowlen) {
    E.cx = rowlen - 1;
    if (E.cx < 0) E.cx = 0;
  }
}

static void process_keypress_normal(char c) {
  if (c == 'q') exit(0);
  else if (c == 'i') E.mode = MODE_INSERT; 
  else if (c == CTRL_KEY('s')) editor_save();
  else if (c == 'x') editor_del_current_char();
  else if (c == 'h' || c == 'j' || c == 'k' || c == 'l') cursor_move(c);
}

static void process_keypress_insert(char c) {
  if (c == 27) E.mode = MODE_NORMAL;
  else if (c == '\r' || c == 13) editor_insert_newline();
  else if (c == 127 || c == 8) editor_del_left_char();
  else editor_insert_char(c);
}

// Handling key input.
void editor_process_keypress(void) {
  char c = editor_readkey(); 

  switch (E.mode) {
    case MODE_NORMAL: process_keypress_normal(c); break;
    case MODE_INSERT: process_keypress_insert(c); break;
    default: break;
  }
}
