#include "input.h"
#include "logger.h"
#include "output.h"
#include "zilo.h"
#include "terminal.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

editor_config_t E;

/**
 * @brief Initialize editor state.
 */
void init_editor(void) {
  E.cx = 0;
  E.cy = 0;

  if (get_window_size(&E.screenrows, &E.screencols) == -1) 
    LOG_WARN("get_window_size", "Unable to obtain terminal size, default value used.");

  E.rowoff = 0;
  E.coloff = 0;

  E.numrows = 0;
  E.row = NULL;

  E.filename = NULL;
  E.mode = MODE_NORMAL;

  editor_set_status_message("Welcome to Zilo.");
}

/**
 * @brief Free editor memory resources.
 */
void free_editor(void) {
  free(E.filename);

  for (int i = 0; i < E.numrows; ++ i) {
    erow_t *row = &E.row[i];
    free(row->chars);
  }

  free(E.row);
}

/**
 * @brief The cleanup function before program exit.
 *
 * This function is registered with 'atexit()' and does not need to be called manually.
 */
void editor_cleanup(void) {
  // Restore the cursor shape to a block
  set_cursor_shape_block();

  // Restore terminal properties
  disable_raw_mode();

  // Free heap memory
  free_editor();

  // Clean terminal
  write(STDOUT_FILENO, ANSI_CLEAR_SCREEN, strlen(ANSI_CLEAR_SCREEN));
  write(STDOUT_FILENO, ANSI_CURSOR_HOME, strlen(ANSI_CURSOR_HOME));

  // Close the log system
  LOG_INFO("", "Zilo editor exited safely.");
  zilo_log_close();
}

// Main logic
int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: zilo <filenname>\n");
    fprintf(stderr, "If the file does not exist, a new file will be created.\n");
    return 1;
  }

  zilo_log_init("zilo.log");
  LOG_INFO("main", "Zilo editor started.");

  char *filenmae = argv[1];

  enable_raw_mode();
  init_editor();
  editor_open(filenmae);

  while (1) {
    editor_refresh_screen();
    editor_process_keypress();
  }
  
  return 0;
}
