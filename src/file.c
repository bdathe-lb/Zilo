#define _POSIX_C_SOURCE 200809L

#include "file.h"
#include "row.h"
#include "zilo.h"
#include "terminal.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Open the file, read its contents, and fill E.row.
 *
 * @param filename File name/path.
 */
void editor_open(char *filename) {
  // Regardless of wether the file exists, 
  // first save the filename to the global configuration.
  free(E.filename);  // In C, free(NULL) is safe
  E.filename = strdup(filename);

  // Try to open the file.
  FILE *fp = fopen(filename, "r");
  
  if (!fp) {
    // The file does not exist, which means we need to create a new file
    if (errno == ENOENT) {
      return;
    }
    // Other error represent failure
    else {
      die("editor_open");
    }
  }


  // If the file exists, read the lines normally
  // Newline characters are not stored; 
  // line breaks are handled by 'draw_rows()'
  char *line = NULL;
  size_t size = 0;
  while (getline(&line, &size, fp) != -1) {
    int len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      len --;
      if (len > 0 && line[len - 1] == '\r') len --;
      line[len] = '\0';
    }

    editor_append_row(line, len);
  }

  // Clean resources
  free(line);
  fclose(fp);
}
