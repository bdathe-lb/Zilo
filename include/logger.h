#ifndef ZILO_LOGGER_H
#define ZILO_LOGGER_H

// Log level
typedef enum {
  LOG_DEBUG = 0,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR
} log_level_e;


// Initialize the log system (open the log file).
void zilo_log_init(const char *filename);

// Close the log system.
void zilo_log_close(void);

// The core print function.
void zilo_log_write(log_level_e level, 
                    const char *func,
                    const char *file, 
                    int line, 
                    const char *fmt, ...);

// External Interface (Macros)
// Use `##VA_ARGS` to handle variadic argguments.
#define LOG_DEBUG(func, fmt, ...) \
  zilo_log_write(LOG_DEBUG, func, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
  
#define LOG_INFO(func, fmt, ...) \
  zilo_log_write(LOG_INFO, func, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_WARN(func, fmt, ...) \
  zilo_log_write(LOG_WARN, func, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(func, fmt, ...) \
  zilo_log_write(LOG_ERROR, func, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif // !ZILO_LOGGER_H
