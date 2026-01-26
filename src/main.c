#include "input.h"
#include "output.h"
#include "zilo.h"
#include "terminal.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>

editor_config_t E;

void init_editor(void) {
  E.cx = 0;
  E.cy = 0;

  if (get_window_size(&E.screenrows, &E.screencols) == -1) die("get_window_size");

  E.rowoff = 0;
  E.coloff = 0;

  E.numrows = 0;
  E.row = NULL;

  E.filename = NULL;
  E.mode = MODE_NORMAL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: zilo <filenname>\n");
    fprintf(stderr, "If the file does not exist, a new file will be created.\n");
    return 1;
  }

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
