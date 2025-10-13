/* The video card, in color text mode. The file is named after IBM's
   Color Graphics Adapter (CGA): later cards, like EGA and VGA, copy its
   text mode, so code written for it works on all of them.

   A video card has its own memory, video memory (VRAM), which the CPU
   reaches like normal memory. Many times a second, the card reads VRAM
   and draws it on the screen. In text mode the screen is a grid of
   cells, 80 across and 25 down, and VRAM holds two bytes per cell: a
   character, and a color byte with its colors.

   In a PC it can:
   - show text in 80 or 40 columns, in 16 text colors on 8 background
     colors; the color byte's top bit makes the cell blink or, if the
     code chooses, gives 16 background colors instead
   - show graphics modes, where each dot is set on its own
   - keep several screens in VRAM (pages) and show any one of them
   - show a blinking cursor at a chosen cell, covering chosen rows of it

   Programs set the card up through the video BIOS: code that comes with
   the PC or the card, called through interrupt 0x10, with the same
   function numbers on every card.

   This file uses the video BIOS to set 80x25 color text mode and to hide
   the cursor, and writes cells straight into VRAM, at address 0xB8000.
   Writing VRAM directly is much faster than asking the BIOS for each
   character. */

#include "hardware\cga.h"
#include <dos.h>
#include <stddef.h>

#define CGA_MEMORY_ADDRESS              0xB8000000L /* segment 0xB800, offset 0: address 0xB8000 */
#define CGA_BYTES_PER_CELL              2           /* the character, then its color byte */
#define CGA_BLINK_SHIFT                 7           /* color byte bit 7: blink */
#define CGA_BACKGROUND_SHIFT            4           /* color byte bits 4-6: background */
#define CGA_BACKGROUND_MASK             0x70        /* color byte bits 4-6 */

#define VIDEO_BIOS_VECTOR               0x10        /* the video BIOS */
#define VIDEO_SET_MODE_FUNCTION         0x00        /* AH = 0x00: set video mode, from AL */
#define VIDEO_COLOR_TEXT_80X25_MODE     0x03        /* mode 3: 80x25 text, 16 colors */
#define VIDEO_SET_CURSOR_SHAPE_FUNCTION 0x01        /* AH = 0x01: set cursor rows, CH to CL */
#define VIDEO_CURSOR_HIDDEN_SHIFT       5           /* CH bits 5-6 = 01: no cursor */

/* VRAM, as a far pointer: it lies outside the program's own memory
   segment, which normal pointers can't leave in the small memory model. */
static byte far *const vram = (byte far *)CGA_MEMORY_ADDRESS;

/* Switches the card to 80x25 color text mode, through the video BIOS.
   This also clears the screen and shows the cursor again. */
void set_text_mode_80x25(void) {
    union REGS regs;
    regs.h.ah = VIDEO_SET_MODE_FUNCTION;
    regs.h.al = VIDEO_COLOR_TEXT_80X25_MODE;
    int86(VIDEO_BIOS_VECTOR, &regs, &regs);
}

/* Hides the blinking text cursor, through the video BIOS. Setting the
   text mode shows it again. */
void hide_cursor(void) {
    union REGS regs;
    regs.h.ah = VIDEO_SET_CURSOR_SHAPE_FUNCTION;

    /* CH holds the hide bit; CL, the cursor's last row, doesn't matter
       while it is hidden. */
    regs.h.ch = 1 << VIDEO_CURSOR_HIDDEN_SHIFT;
    regs.h.cl = 0;
    int86(VIDEO_BIOS_VECTOR, &regs, &regs);
}

/* Packs blink and the two colors into a cell's color byte:

       bit   7       6 5 4        3 2 1 0
             blink   background   foreground

   The background gets only 3 bits, so colors 8-15 lose their top bit
   there (DARK_GRAY becomes BLACK). */
static byte encode_color_byte(bool blink, byte background, byte foreground) {
    byte colors = ((background << CGA_BACKGROUND_SHIFT) & CGA_BACKGROUND_MASK) | foreground;

    return blink ? colors | (1 << CGA_BLINK_SHIFT) : colors;
}

/* Writes a character and its colors straight into VRAM, at the cell in
   row and col; it shows on the screen right away. */
void vram_write(int row, int col, unsigned char chr,
                bool blink, byte background, byte foreground) {
    size_t offset = CGA_BYTES_PER_CELL * (row * CGA_TEXT_COLUMNS + col);
    vram[offset] = chr;
    vram[offset + 1] = encode_color_byte(blink, background, foreground);
}
