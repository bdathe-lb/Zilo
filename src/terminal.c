#include "terminal.h"
#include "logger.h"
#include "zilo.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

/**
 * @brief Enable terminal 'Raw' mode.
 *
 * Logic:
 *  1. Save the current attributes
 *  2. Register 'atexit(disable_raw_mode)'
 *  3. Copy the configuration and modify it
 *  4. Apply new settings
 */
void enable_raw_mode(void) {
  struct termios raw;
  int ret;

  // 1. Save the current attributes
  ret = tcgetattr(STDIN_FILENO, &E.orig_termios);
  if (ret == -1) {
    LOG_ERROR("tcsetattr", "Failed to get the current terminal attributes.");
    return;
  }

  // 2. Register 'atexit(disable_raw_mode)'
  ret = atexit(editor_cleanup);
  if (ret != 0) {
    LOG_ERROR("atexit", "Failed to register cleanup function.");
    return;
  }

  // 3. Copy the configuration and modify it
  raw = E.orig_termios;
  raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

  // VMIN > 0, VTIME = 0 —— "Return only after reading N items" (pure blocking)
  // VMIN = 0, VTIME = 0 —- "Completely non-blocking" (polling)
  // VMIN = 0, VTIME > 0 —— "Timed wait" (overall timeout)

  // Wait a maximum of 100ms; return 0 if no key is pressed
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  // 4. Apply new settings
  ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  if (ret == -1) {
    LOG_ERROR("tcsetattr", "Failed to set terminal attributes.");
    return;
  }
}

/**
 * @brief Disable terminal 'Raw' mode.
 */
void disable_raw_mode(void) {
  // TCSAFLUSH: Discard input that has not yet been read,
  //            then apply the new attributes.
  int ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios);
  if (ret == -1) {
    LOG_ERROR("tcsetattr", "Failed to restore terminal attributes.");
    return;
  }
}

/**
 * @brief Set the cursor shape to block.
 */
void set_cursor_shape_block(void) {
  int ret = write(STDOUT_FILENO, ANSI_CURSOR_SHAPE_BLOCK, strlen(ANSI_CURSOR_SHAPE_BLOCK));
  if (ret == -1) {
    LOG_ERROR("write", "Failed to set cursor shape to block.");
    return;
  }
}

/**
 * @brief Read a key.
 *
 * @return Returns char from standard input.
 */
char editor_readkey(void) {
  char c;

  while (1) {
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n == -1 && errno != EAGAIN) LOG_ERROR("read", "Failed to read key.");
    if (n == 0) continue;
    if (n == 1) break;
  }

  return c;
}

/**
 * @brief Get the window size of terminal.
 *
 * @param rows Stored row.
 * @param cols Stored columns.
 *
 * @return Returns 0 if success, otherwise returns -1.
 */
int get_window_size(int *rows, int *cols) {
  struct winsize ws;
  int ok = 0;

  int fds[] = { STDOUT_FILENO, STDIN_FILENO, STDERR_FILENO };

  for (int i = 0; i < 3; ++ i) {
    if (isatty(fds[i]) &&
        ioctl(fds[i], TIOCGWINSZ, &ws) == 0 &&
        ws.ws_col > 0) {
      ok = 1;
      break;
    }
  }

  if (!ok) {
    int tty = open("/dev/tty", O_RDONLY);
    if (tty >= 0 &&
        ioctl(tty, TIOCGWINSZ, &ws) == 0) {
      ok = 1;
    }
    close(tty);
  }

  if (!ok) {
    // Set a default value
    *rows = 80;
    *cols = 60;
    return -1;
  }

  *rows = ws.ws_row - 1; // The bottom should be reserved for the status bar
  *cols = ws.ws_col;
  return 0;
}
