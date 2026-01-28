#ifndef ZILO_ZILO_H
#define ZILO_ZILO_H

#include <termios.h>

typedef enum {
  MODE_NORMAL,
  MODE_INSERT
} editor_mode_e;

typedef struct {
  int size;       // Record how many bytes this line contains
  char *chars;    // Pointer to the actual character data (Does not contain \r\n)
} erow_t;

typedef struct {
  int cx, cy;     // The logical position of the cursor in the file (Cursor X, Y)
  
  int rowoff;     // The first line of the screen corresponds to which line of the file (for vertical scrolling)
  int coloff;     // The first column of the screen corresponds to which column of the file (for horizontal scrolling)

  int screenrows; // Terminal row number
  int screencols; // Terminal column number

  int numrows;    // The total row number of file
  erow_t *row;    // The row array pointer

  char *filename;               // The currently opened file (heap memory)
  editor_mode_e mode;           // The current mode
  struct termios orig_termios;  // Save the original state when the terminal exists
} editor_config_t;

extern editor_config_t E;

// Initialize editor state.
void init_editor(void);

// Free editor resources.
void free_editor(void);

// The cleanup function before program exit.
void editor_cleanup(void);

#endif // !ZILO_ZILO_H
