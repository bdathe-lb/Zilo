#ifndef ZILO_INPUT_H
#define ZILO_INPUT_H

#define CTRL_KEY(k) ((k) & 0x1f)

// Handling key input.
void editor_process_keypress(void);

#endif // !ZILO_INPUT_H
