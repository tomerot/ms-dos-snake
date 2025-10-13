#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "hardware\keyboard.h"

/* What read_key_press returns when no key was pressed; no key has scan
   code 0. */
#define NO_KEY 0

/* A key, by its scan code: one of the KEY_ names in keyboard.h. */
typedef byte key;

void start_input(void);
void stop_input(void);

key read_key_press(void);
bool is_key_held(key k);

#endif
