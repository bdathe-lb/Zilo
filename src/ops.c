#include "ops.h"
#include "file.h"
#include "row.h"
#include "zilo.h"
#include "edit.h"
#include <stdlib.h>

// q
void editor_op_exit(void) {
  exit(1);
}

// Ctrl+s
void editor_op_save_file(void) {
  editor_save();
}

// 0
void editor_op_return_bol(void) {
  if (E.cy < 0 || E.cy >= E.numrows) return;

  E.cx = 0;
}

// $
void editor_op_return_eol(void) {
  // Get the current row object
  if (E.cy < 0 || E.cy >= E.numrows) return;
  erow_t *row = &E.row[E.cy];

  E.cx = row->size - 1;
}

// dd
void editor_op_delete_current_row(void) {
  if (E.pending_key == 0) {
    E.pending_key = 'd';
    return;
  }

  if (E.pending_key == 'd') {
    editor_del_row(E.cy);

    E.pending_key = 0;
  }
}

// gg
void editor_op_goto_top(void) {
  if (E.pending_key == 0) {
    E.pending_key = 'g';
    return;
  }

  if (E.pending_key == 'g') {
    E.cy = 0;

    erow_t *row = &E.row[0];

    if (E.cx > row->size) E.cx = row->size;

    E.pending_key = 0;
  }
}

// G
void editor_op_goto_bottom(void) {
  E.cy = E.numrows - 1;

  erow_t *row = &E.row[E.numrows - 1];

  if (E.cx > row->size) E.cx = row->size;
}

// x
void editor_op_del_current_char(void) {
  editor_del_current_char();
}

// o
void editor_op_open_below(void) {
  editor_insert_row(E.cy + 1, "", 0); 
  E.cy ++; 
  E.cx = 0; 
  E.mode = MODE_INSERT;
}

// O
void editor_op_open_above(void) {
  editor_insert_row(E.cy, "", 0); 
  E.cx = 0; 
  E.mode = MODE_INSERT;
}

// A
void editor_op_append_eol(void) {
  if (E.cy >= E.numrows) return;
  
  erow_t *row = &E.row[E.cy];

  E.cx = row->size;
  E.mode = MODE_INSERT;
}

// I
void editor_op_insert_bol(void) {
  if (E.cy >= E.numrows) return;

  E.cx = 0; 
  E.mode = MODE_INSERT; 
}

// enter
void editor_op_insert_newline(void) {
  editor_insert_newline();
}

// backspace
void editor_op_del_left_char(void) {
  editor_del_left_char(); 
}

// L-VISUAL d/x
void editor_op_delete_visual_line(void) {
  int start_y = MIN(E.select_cy, E.cy);
  int end_y = MAX(E.select_cy, E.cy);

  // NOTE: If deleting from beginning to end: for (int i = start_y; i <= end_y; ++ i) ...
  // Problems may occur:
  // Suppose you want to delete rows 1, 2, and 3;
  // i=1: Delete line 1. The original line 2 becomes the current line 1;
  // The loop increments i to 2.
  // i=2: Delete the current second line (which is actually the original third line).
  // Result: The original second line was skipped and not deleted.

  // Delete from back to front
  for (int i = end_y; i >= start_y; -- i) {
    editor_del_row(i);
  }

  // Cleanup work after deletion
  E.mode = MODE_NORMAL;
  E.cy = start_y;
  if (E.cy >= E.numrows) E.cy = E.numrows - 1;
  if (E.cy < 0) E.cy = 0;

  erow_t *row = (E.cy < E.numrows) ? &E.row[E.cy] : NULL;
  int sz = row ? row->size : 0;
  if (E.cx > sz) E.cx = sz;
}

// B-VISUAL d/x
void editor_op_delete_visual_block(void) {
  int start_y = MIN(E.select_cy, E.cy);
  int end_y = MAX(E.select_cy, E.cy);
  int start_x = MIN(E.select_cx, E.cx);
  int end_x = MAX(E.select_cx, E.cx);

  for (int i = end_y; i >= start_y; -- i) {
    erow_t *row = &E.row[i];
    int len_to_delete = end_x - start_x + 1;
    editor_row_remove_range(row, start_x, len_to_delete);
  }

  // Cleanup work after deletion
  E.mode = MODE_NORMAL;
  E.cy = start_y;
  E.cx = start_x;
  if (E.cy >= E.numrows) E.cy = E.numrows - 1;
  if (E.cy < 0) E.cy = 0;

  erow_t *row = (E.cy < E.numrows) ? &E.row[E.cy] : NULL;
  int sz = row ? row->size : 0;
  if (E.cx > sz) E.cx = sz;
}

// VISUAL d/x
void editor_op_delete_visual(void) {
  int start_y = MIN(E.cy, E.select_cy);
  int end_y = MAX(E.cy, E.select_cy);

  int start_x, end_x;
  if (E.cy < E.select_cy) {
    start_x = E.cx;
    end_x = E.select_cx;
  } else if (E.cy == E.select_cy) {
    start_x = MIN(E.cx, E.select_cx);
    end_x = MAX(E.cx, E.select_cx);
  } else {
    start_x = E.select_cx;
    end_x = E.cx;
  }

  // Case A: Single row deletion
  // Delete [start_x, end_x]
  if (start_y == end_y) {
    erow_t *row = &E.row[start_y];
    int len_to_delete = end_x - start_x + 1;
    if (len_to_delete == row->size) {
      editor_del_row(start_y);
    } else {
      editor_row_remove_range(row, start_x, len_to_delete);
    }
  } 
  // Case B: Deleting multiple lines
  else {
    erow_t *start_row = &E.row[start_y];
    erow_t *end_row = &E.row[end_y];

    // 1. Calculate the "tail" (the content after end_x) that needs 
    // to be retained in the last line.
    // Note that `end_x` must be checked to ensure it is the end of the line. 
    // If it is, the tail length is 0.
    char *tail = end_row->chars + end_x + 1;
    int tail_len = end_x == end_row->size - 1 
                   ? 0 
                   : end_row->size - end_x - 1;

    // 2. Truncate the first line to `start_x`
    start_row->size = start_x;

    // 3. Add "tail" to the end of the first line
    editor_row_append_string(start_row, tail, tail_len);

    // 4. Starting from `start_y + 1`, delete all subsequent lines involving this value
    for (int i = end_y; i >= start_y + 1; -- i) {
      editor_del_row(i);
    }
  }

  // Cleanup work after deletion
  E.mode = MODE_NORMAL;
  E.cy = start_y;
  E.cx = start_x;
  if (E.cy >= E.numrows) E.cy = E.numrows - 1;
  if (E.cy < 0) E.cy = 0;

  erow_t *row = (E.cy < E.numrows) ? &E.row[E.cy] : NULL;
  int sz = row ? row->size : 0;
  if (E.cx > sz) E.cx = sz;
}
