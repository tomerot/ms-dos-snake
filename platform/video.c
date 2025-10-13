#include "platform\video.h"
#include <string.h>

#define GLYPH_BLOCK 219  /* a full block, in the PC's character set */

#define LETTER_WIDTH  5
#define LETTER_HEIGHT 5
#define LETTER_GAP    2

/* The big-letter font, A to Z: each letter is LETTER_HEIGHT rows of
   LETTER_WIDTH, with # for a piece and . for nothing. */
static const char *BIG_LETTERS['Z' - 'A' + 1][LETTER_HEIGHT] = {
    {   /* A */
        "#####",
        "#...#",
        "#####",
        "#...#",
        "#...#"
    },
    {   /* B */
        "####.",
        "#...#",
        "####.",
        "#...#",
        "####."
    },
    {   /* C */
        "#####",
        "#....",
        "#....",
        "#....",
        "#####"
    },
    {   /* D */
        "####.",
        "#...#",
        "#...#",
        "#...#",
        "####."
    },
    {   /* E */
        "#####",
        "#....",
        "#####",
        "#....",
        "#####"
    },
    {   /* F */
        "#####",
        "#....",
        "####.",
        "#....",
        "#...."
    },
    {   /* G */
        "#####",
        "#....",
        "#..##",
        "#...#",
        "#####"
    },
    {   /* H */
        "#...#",
        "#...#",
        "#####",
        "#...#",
        "#...#"
    },
    {   /* I */
        "#####",
        "..#..",
        "..#..",
        "..#..",
        "#####"
    },
    {   /* J */
        "#####",
        "...#.",
        "...#.",
        "#..#.",
        "####."
    },
    {   /* K */
        "#...#",
        "#..#.",
        "###..",
        "#..#.",
        "#...#"
    },
    {   /* L */
        "#....",
        "#....",
        "#....",
        "#....",
        "#####"
    },
    {   /* M */
        "#####",
        "#.#.#",
        "#...#",
        "#...#",
        "#...#"
    },
    {   /* N */
        "#...#",
        "##..#",
        "#.#.#",
        "#..##",
        "#...#"
    },
    {   /* O */
        "#####",
        "#...#",
        "#...#",
        "#...#",
        "#####"
    },
    {   /* P */
        "#####",
        "#...#",
        "#####",
        "#....",
        "#...."
    },
    {   /* Q */
        "#####",
        "#...#",
        "#.#.#",
        "#..##",
        "#####"
    },
    {   /* R */
        "#####",
        "#...#",
        "#####",
        "#..#.",
        "#...#"
    },
    {   /* S */
        "#####",
        "#....",
        "#####",
        "....#",
        "#####"
    },
    {   /* T */
        "#####",
        "..#..",
        "..#..",
        "..#..",
        "..#.."
    },
    {   /* U */
        "#...#",
        "#...#",
        "#...#",
        "#...#",
        "#####"
    },
    {   /* V */
        "#...#",
        "#...#",
        ".#.#.",
        ".#.#.",
        "..#.."
    },
    {   /* W */
        "#...#",
        "#...#",
        "#...#",
        "#.#.#",
        "#####"
    },
    {   /* X */
        "#...#",
        ".#.#.",
        "..#..",
        ".#.#.",
        "#...#"
    },
    {   /* Y */
        "#...#",
        ".#.#.",
        "..#..",
        "..#..",
        "..#.."
    },
    {   /* Z */
        "#####",
        "...#.",
        "..#..",
        ".#...",
        "#####"
    }
};

/* Writes one character at p, in the given colors, without blinking. */
static void set_char(pos p, unsigned char c, color background, color foreground) {
    vram_write(p.y, p.x, c, FALSE, background, foreground);
}

/* Draws one big letter with its top-left corner at corner: a piece for
   each # in its rows. */
static void set_big_letter(pos corner, const char **rows, color fill) {
    pos p;
    int x, y;

    for (y = 0; y < LETTER_HEIGHT; y++)
        for (x = 0; x < LETTER_WIDTH; x++)
            if (rows[y][x] == '#') {
                p.y = corner.y + y;
                p.x = corner.x + x;
                set_piece(p, fill);
            }
}

/* Switches the screen to 80x25 color text mode, clears it, and hides the
   cursor. */
void start_video(void) {
    set_text_mode_80x25();
    hide_cursor();
}

/* Gives the screen back to DOS: setting the text mode again clears it and
   shows the cursor. */
void stop_video(void) {
    set_text_mode_80x25();
}

/* Draws a piece at p: a full block in the fill color. The block covers
   the whole character, so its background never shows. */
void set_piece(pos p, color fill) {
    set_char(p, GLYPH_BLOCK, BLACK, fill);
}

/* Writes text from p to the right, one character per position. The text
   must fit on the line; past the right edge, it goes on at the start of
   the next one. */
void set_text(pos p, const char *text, color background, color foreground) {
    int i;

    for (i = 0; text[i] != '\0'; i++) {
        set_char(p, text[i], background, foreground);
        p.x++;
    }
}

/* Draws text in big letters, with p as the first letter's top-left
   corner and LETTER_GAP positions between letters. Only A to Z are drawn;
   any other character, like a space, leaves an empty letter's width. */
void set_big_text(pos p, const char *text, color fill) {
    int i;

    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] >= 'A' && text[i] <= 'Z')
            set_big_letter(p, BIG_LETTERS[text[i] - 'A'], fill);
        p.x += LETTER_WIDTH + LETTER_GAP;
    }
}

/* How many positions across text takes in big letters, from the first
   letter's left edge to the last one's right edge. */
int big_text_width(const char *text) {
    int letters = strlen(text);

    return (letters == 0) ? 0 : letters * (LETTER_WIDTH + LETTER_GAP) - LETTER_GAP;
}
