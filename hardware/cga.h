#ifndef CGA_H
#define CGA_H

#include "common.h"

#define CGA_TEXT_ROWS    25  /* cells down, in 80x25 text mode */
#define CGA_TEXT_COLUMNS 80  /* cells across */

enum {
    BLACK         = 0,
    BLUE          = 1,
    GREEN         = 2,
    CYAN          = 3,
    RED           = 4,
    MAGENTA       = 5,
    BROWN         = 6,
    LIGHT_GRAY    = 7,
    DARK_GRAY     = 8,
    LIGHT_BLUE    = 9,
    LIGHT_GREEN   = 10,
    LIGHT_CYAN    = 11,
    LIGHT_RED     = 12,
    LIGHT_MAGENTA = 13,
    YELLOW        = 14,
    WHITE         = 15
};

void set_text_mode_80x25(void);
void hide_cursor(void);

void vram_write(int row, int col, unsigned char chr,
                bool blink, byte background, byte foreground);

#endif
