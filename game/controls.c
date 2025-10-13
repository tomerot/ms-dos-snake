#include "game\controls.h"
#include "platform\input.h"

/* Reads the latest key press as a command: the arrow keys steer, ENTER
   restarts and ESC quits. */
command read_command(void) {
    switch (read_key_press()) {
        case KEY_UP:    return GO_UP;
        case KEY_DOWN:  return GO_DOWN;
        case KEY_LEFT:  return GO_LEFT;
        case KEY_RIGHT: return GO_RIGHT;
        case KEY_ENTER: return RESTART;
        case KEY_ESC:   return QUIT;
        default:        return NO_COMMAND;
    }
}
