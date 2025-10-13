#ifndef STYLE_H
#define STYLE_H

#include "platform\video.h"

#define FIELD_DARK_COLOR        GREEN       /* dark squares of the field's checkerboard */
#define FIELD_BRIGHT_COLOR      LIGHT_GREEN /* bright squares of the field's checkerboard */
#define SNAKE_COLOR             BLUE
#define FOOD_COLOR              MAGENTA
#define GLOSSY_COLOR            YELLOW      /* the food, for a moment after it is eaten */
#define WALL_COLOR              BROWN

/* The HUD (head-up display): the top line, with the seconds left and the
   level. */
#define HUD_BACKGROUND_COLOR    BROWN
#define HUD_TEXT_COLOR          WHITE

/* The game-over screen: the big title and the prompt under it. */
#define TITLE_COLOR             RED
#define PROMPT_BACKGROUND_COLOR RED
#define PROMPT_TEXT_COLOR       WHITE

#endif
