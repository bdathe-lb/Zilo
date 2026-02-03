#include "input.h"
#include "edit.h"
#include "ops.h"
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
      if (row && E.cx < row->size - 1) E.cx ++;
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
  if      (c == 'q')            { editor_op_exit(); }
  else if (c == 'i')            { E.mode = MODE_INSERT; }
  else if (c == 'r')            { E.mode = MODE_REPLACE_ONCE; }
  else if (c == 'R')            { E.mode = MODE_REPLACE; }
  else if (c == 'v')            { E.mode = MODE_VISUAL; E.select_cy = E.cy; E.select_cx = E.cx; }
  else if (c == 'V')            { E.mode = MODE_VISUAL_LINE; E.select_cy = E.cy; }
  else if (c == CTRL_KEY('v'))  { E.mode = MODE_VISUAL_BLOCK;E.select_cy = E.cy; E.select_cx = E.cx; }
  else if (c == '0')            { editor_op_return_bol(); }
  else if (c == '$')            { editor_op_return_eol(); }
  else if (c == 'd')            { editor_op_delete_current_row(); }
  else if (c == 'g')            { editor_op_goto_top(); }
  else if (c == 'G')            { editor_op_goto_bottom(); }
  else if (c == 'x')            { editor_op_del_current_char(); }
  else if (c == 'o')            { editor_op_open_below(); }
  else if (c == 'O')            { editor_op_open_above(); }
  else if (c == 'A')            { editor_op_append_eol(); }
  else if (c == 'I')            { editor_op_insert_bol(); }
  else if (c == CTRL_KEY('s'))  { editor_op_save_file(); }
  else if (c == 'h' ||
           c == 'j' ||
           c == 'k' ||
           c == 'l')            { cursor_move(c); }
}

static void process_keypress_insert(char c) {
  if (c == 27)            { E.mode = MODE_NORMAL; }
  else if (c == '\r' ||
           c == 13)       { editor_op_insert_newline(); }
  else if (c == 127  ||
           c == 8)        { editor_op_del_left_char(); }
  else                    { editor_insert_char(c); }
}

static void process_keypress_replace(char c) {
  // Press Esc, do nothing, and it will directly return to Normal Mode
  if (c == 27) {
    E.mode = MODE_NORMAL;
    return;
  }

  // Get the current row
  if (E.cy >= E.numrows) {
    E.mode = MODE_NORMAL;
  }

  erow_t *row = &E.row[E.cy];
 
  // If the cursor position exceeds the line length, it cannot be replaced
  if (E.cx >= row->size) {
    E.mode = MODE_NORMAL;
    return;
  }

  row->chars[E.cx] = c;

  E.cx ++;
}

static void process_keypress_replace_only(char c) {
  // Press Esc, do nothing, and it will directly return to Normal Mode
  if (c == 27) {
    E.mode = MODE_NORMAL;
    return;
  }

  // Get the current row
  if (E.cy >= E.numrows) {
    E.mode = MODE_NORMAL;
  }

  erow_t *row = &E.row[E.cy];

  // If the cursor position exceeds the line length, it cannot be replaced
  if (E.cx >= row->size) {
    E.mode = MODE_NORMAL;
    return;
  }

  row->chars[E.cx] = c;

  E.mode = MODE_NORMAL;
}

static void process_keypress_visual(char c) {
  // Press Esc, do nothing, and it will directly return to Normal Mode
  if (c == 27) {
    E.mode = MODE_NORMAL;
    return;
  }
  else if (c == 'h' || c == 'j' ||
           c == 'k' || c == 'l')   { cursor_move(c); }
  else if (c == 'd' || c == 'x')   { editor_op_delete_visual(); }
}

static void process_keypress_visual_line(char c) {
  // Press Esc, do nothing, and it will directly return to Normal Mode
  if (c == 27) {
    E.mode = MODE_NORMAL;
    return;
  }
  else if (c == 'h' || c == 'j' ||
           c == 'k' || c == 'l')   { cursor_move(c); }
  else if (c == 'd' || c == 'x')   { editor_op_delete_visual_line(); }
}

static void process_keypress_visual_block(char c) {
  // Press Esc, do nothing, and it will directly return to Normal Mode
  if (c == 27) {
    E.mode = MODE_NORMAL;
    return;
  }
  else if (c == 'h' || c == 'j' ||
           c == 'k' || c == 'l')   { cursor_move(c); }
  else if (c == 'd' || c == 'x')   { editor_op_delete_visual_block(); }
}

// Handling key input.
void editor_process_keypress(void) {
  char c = editor_readkey();

  switch (E.mode) {
    case MODE_NORMAL:        process_keypress_normal(c);       break;
    case MODE_INSERT:        process_keypress_insert(c);       break;
    case MODE_REPLACE:       process_keypress_replace(c);      break;
    case MODE_REPLACE_ONCE:  process_keypress_replace_only(c); break;
    case MODE_VISUAL:        process_keypress_visual(c);       break;
    case MODE_VISUAL_LINE:   process_keypress_visual_line(c);  break;
    case MODE_VISUAL_BLOCK:  process_keypress_visual_block(c); break;
    default: break;
  }
}
