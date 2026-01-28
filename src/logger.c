#define _POSIX_C_SOURCE 200809L

#include "logger.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

static FILE *g_log_fp = NULL;
static log_level_e g_log_level = LOG_DEBUG;

static const char *level_strings[] = {
  "DEBUG", "INFO", "WARN", "ERROR"
};

/**
 * @brief Initialize the log system (open the log file).
 *
 * @param filename File name/path.
 */
void zilo_log_init(const char *filename) {
  if (g_log_fp) fclose(g_log_fp);

  g_log_fp = fopen(filename, "a");
  if (!g_log_fp) {
    fprintf(stderr, "The log file <%s> failed to open.\n", filename);
    return;
  }
}

/**
 * @brief Close the log system.
 */
void zilo_log_close(void) {
  if (g_log_fp) {
    fclose(g_log_fp);
    g_log_fp = NULL;
  }
}

/**
 * @brief Get the current time store to buffer.
 *
 * @param time_buf Buffer.
 * @param buf_size Buffer size.
 *
 * @return Returns 0 if success, otherwise returns -1.
 */
static int get_current_time(char *time_buf, size_t buf_size) {
  struct timeval tv;

  // Get high-precision timestamp
  if (gettimeofday(&tv, NULL) == -1) return -1;

  // Time conversion
  struct tm tm;
  if (localtime_r(&tv.tv_sec, &tm) == NULL) return -1;

  // Convert time structure to string
  size_t len = strftime(time_buf, buf_size, 
                "%Y-%m-%d %H:%M:%S", &tm);
  if (len == 0) return -1;

  return 0;
}

/**
 * @brief The core print function.
 *
 * @param level The log level.
 * @param func  The function that generated this log.
 * @param file  The file that generated this log.
 * @param line  The line that generated this log.
 * @param fmt   Additional output information.
 */
void zilo_log_write(log_level_e level, 
                    const char *func,
                    const char *file, 
                    int line, 
                    const char *fmt, ...) {
  if (!g_log_fp) return;
  if (level < g_log_level) return;

  // Get the current time.
  char time_buf[64];
  if (get_current_time(time_buf, 64) == -1) return;

  // Format log header: [Time] [Level] File:Line -> <Func> --
  fprintf(g_log_fp, "[%s] [%-5s] %s:%d -> <%s> -- ",
          time_buf, 
          level_strings[level],
          file,
          line,
          func);

  // Format user message
  va_list args;
  va_start(args, fmt);
  vfprintf(g_log_fp, fmt, args);
  va_end(args);
  
  // Newline and flush buffer
  fprintf(g_log_fp, "\n");
  fflush(g_log_fp);
}


