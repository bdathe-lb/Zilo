#ifndef ZILO_OPS_H
#define ZILO_OPS_H

/*------------------------------------------
              NORMAL MODE 
 ------------------------------------------*/
// q
void editor_op_exit(void);

// Ctrl+s
void editor_op_save_file(void);

// 0
void editor_op_return_bol(void);

// $
void editor_op_return_eol(void);

// dd
void editor_op_delete_current_row(void);

// gg
void editor_op_goto_top(void);

// G
void editor_op_goto_bottom(void);

// x
void editor_op_del_current_char(void);

// o
void editor_op_open_below(void);

// O
void editor_op_open_above(void);

// A
void editor_op_append_eol(void);

// I
void editor_op_insert_bol(void);

/*------------------------------------------
              INSERT MODE 
 ------------------------------------------*/
// enter
void editor_op_insert_newline(void);

// backspace
void editor_op_del_left_char(void);

/*------------------------------------------
              VISUAL MODE 
 ------------------------------------------*/

void editor_op_delete_visual_block(void);

void editor_op_delete_visual_line(void);

void editor_op_delete_visual(void);

#endif // !ZILO_OPS_H
