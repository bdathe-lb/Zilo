#include "terminal.h"
#include "zilo.h"
#include <asm-generic/ioctls.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

/**
 * @brief Prints error messages and exists the program.
 *
 * @param s Location of the error.
 */
void die(const char *s) {
  if (!s) return;

  perror(s);
  exit(1);
}

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
  if (ret != 0) die("tcgetattr");

  // 2. Register 'atexit(disable_raw_mode)'
  ret = atexit(disable_raw_mode);
  if (ret != 0) die("atexit");

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
  if (ret != 0) die("tcsetattr");
}

/**
 * @brief Disable terminal 'Raw' mode.
 */
void disable_raw_mode(void) {
  // TCSAFLUSH: Discard input that has not yet been read,
  //            then apply the new attributes.
  int ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios);
  if (ret != 0) die("tcsetattr");
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
    if (n == -1 && errno != EAGAIN) die("read");
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

  if (!ok) return -1;

  *rows = ws.ws_row;
  *cols = ws.ws_col;
  return 0;
}
