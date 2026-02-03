#ifndef ZILO_EDIT_H
#define ZILO_EDIT_H

#include <stddef.h>

// Insert characters at the current cursor position 
// (handles automatic line wrapping and cursor movement).
void editor_insert_char(int c);

// Insert a new blank line at position `at`.
void editor_insert_row(int at, char *s, size_t len);

// Insert a newline character at the current cursor position (Enter key logic).
void editor_insert_newline(void);

// Delete the character to the left of the current cursor (Backspace logic).
void editor_del_left_char(void);

// Delete the current cursor position character (Normal mode 'x' logic).
void editor_del_current_char(void);

// Delete the specify row. (Normal mdoe 'dd' logic)
void editor_del_row(int at);

#endif // !ZILO_EDIT_H
