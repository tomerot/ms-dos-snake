#ifndef CONTROLS_H
#define CONTROLS_H

/* What a key asks the game to do. NO_COMMAND means no key was pressed, or
   one the game doesn't use. */
typedef enum { NO_COMMAND, GO_UP, GO_DOWN, GO_LEFT, GO_RIGHT, RESTART, QUIT } command;

command read_command(void);

#endif
