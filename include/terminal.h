#ifndef ZILO_TERMINAL_H
#define ZILO_TERMINAL_H

#define ANSI_CLEAR_SCREEN       "\x1b[2J"         // Clear screen
#define ANSI_CURSOR_HOME        "\x1b[H"          // Move the cursor back to the top left corner
#define ANSI_CURSOR_HIDE        "\x1b[?25l"       // Hide cursor
#define ANSI_CURSOR_SHOW        "\x1b[?25h"       // Show cursor
#define ANSI_CLEAR_LINE         "\x1b[K"          // Clear the content from the cursor to the end of the line
#define ANSI_REVERSE_DISPLAY    "\x1b[7m"         // Enable reverse dislplay
#define ANSI_RESET              "\x1b[m"          // Reset attributes
#define ANSI_CURSOR_SHAPE_BLOCK "\x1b[2 q"        // Cursor shape (Block)
#define ANSI_CURSOR_SHAPE_BAR   "\x1b[6 q"        // Cursor shape (Bar)

// Enable terminal 'Raw' mode.
void enable_raw_mode(void);

// Disable terminal 'Raw' mode.
void disable_raw_mode(void);

// Set the cursor shape to block.
void set_cursor_shape_block(void);

// Read a key.
char editor_readkey(void);

// Get window size of terminal.
int get_window_size(int *rows, int *cols);

#endif // !ZILO_TERMINAL_H
