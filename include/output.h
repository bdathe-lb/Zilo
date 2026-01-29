#ifndef ZILO_OUTPUT_H
#define ZILO_OUTPUT_H

// Clears the screen, draws a tilde, moves the cursor, 
// and finally refreshes the screen.
void editor_refresh_screen(void);

// Set status message.
void editor_set_status_message(const char *fmt, ...);

#endif // !ZILO_OUTPUT_H
