#include "output.h"
#define _POSIX_C_SOURCE 200809L

#include "file.h"
#include "row.h"
#include "zilo.h"
#include <fcntl.h>
#include "logger.h"
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/**
 * @brief Concatenate the contents of all row objects into a single large string (used for save).
 *
 * @return String pointer.
 */
static void all_row_to_string(char **s, size_t *len) {
  // Calculate the total bytes
  size_t bytes = 0;
  for (int i = 0; i < E.numrows; ++ i) {
    bytes += E.row[i].size + 1; // An additional 1 byte is required to store '\n'
  }

  // Allocate memory space
  char *buf = malloc(sizeof(char) * bytes);
  if (!buf) LOG_ERROR("malloc", "Failed to allocate memory.");
  
  // Concatenate all rows
  size_t p = 0;
  for (int i = 0; i < E.numrows; ++ i) {
    erow_t *row = &E.row[i];
    // Copy
    memcpy(buf + p, row->chars, row->size);
    p += row->size;

    buf[p ++] = '\n';
  }

  *s = buf;
  *len = p;
}

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
      LOG_ERROR("open", "Failed to open the file <%s>.", E.filename);
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


/**
 * @brief Save the edited content.
 */
void editor_save(void) {
  char *buf;
  size_t len;

  all_row_to_string(&buf, &len);

  // Do not truncate when opening; preserve the original content
  int fd = open(E.filename,
                O_WRONLY | O_CREAT, 
                0644);
  if (fd == -1) LOG_ERROR("open", "Failed to open the file <%s>.", E.filename);

  // Use 'ftruncate()' to truncate
  ftruncate(fd, len);

  // Write to disk
  if (write(fd, buf, len) == -1) LOG_ERROR("write", "Failed to write data to file <%s>.", E.filename);

  // Set a message 
  editor_set_status_message("The file <%s> has been saved to disk.", E.filename);

  free(buf);
  close(fd);
}
