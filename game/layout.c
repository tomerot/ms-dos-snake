#include "game\layout.h"
#include "game\style.h"
#include "platform\video.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* The layout fills the whole screen: the border is its outer ring, and the
   HUD is written over the border's top line. */
#define LAYOUT_WIDTH  CGA_TEXT_COLUMNS
#define LAYOUT_HEIGHT CGA_TEXT_ROWS

#define FIELD_SQUARE_SIZE 4  /* side of the field's checkerboard squares */

#define SNAKE_LENGTH       9  /* in pieces; the snake never grows */
#define SNAKE_TAIL         0
#define SNAKE_HEAD         (SNAKE_LENGTH - 1)
#define SNAKE_START_Y      (LAYOUT_HEIGHT / 2)
#define SNAKE_START_TAIL_X (LAYOUT_WIDTH / 4)

#define WALL_ARM_LENGTH 4  /* pieces in each arm of the wall's V, after the tip */

#define SAFE_DISTANCE 10  /* fewest steps from the head to a new wall's tip */

/* The HUD: each label starts at its _X on line HUD_Y, with its number right
   after it. */
#define HUD_Y             0
#define HUD_SECONDS_X     28
#define HUD_LEVEL_X       44
#define HUD_SECONDS_LABEL "SEC LEFT: "
#define HUD_LEVEL_LABEL   "LEVEL: "

#define GAME_OVER_TITLE    "GAME OVER"  /* in big letters */
#define GAME_OVER_TITLE_Y  8
#define GAME_OVER_PROMPT   "  Restart <ENTER>  \x07    Exit <ESC>     "  /* \x07 is a round dot; the spaces widen its bar and center the dot in it */
#define GAME_OVER_PROMPT_Y 14

/* Any of the set_<item>_piece functions, so fill_food and fill_wall can
   draw with each of them. */
typedef void (*generic_set)(pos p);

/* A rectangle of positions, from its top-left to its bottom-right corner,
   both included. */
typedef struct {
    pos top_left;
    pos bottom_right;
} area;

/* The snake's pieces, from the tail at snake[SNAKE_TAIL] to the head at
   snake[SNAKE_HEAD]. */
static pos snake[SNAKE_LENGTH];

/* The food: a food_size by food_size square, its top-left corner at
   food_corner. */
static pos food_corner;
static int food_size;

/* The wall: a V lying on its side, its tip at wall_tip pointing
   wall_direction, toward the snake, and its open side around the food. */
static pos wall_tip;
static direction wall_direction;

/* TRUE if position p is on a dark square of the field's checkerboard. */
static bool is_dark_square(pos p) {
    return (p.x / FIELD_SQUARE_SIZE + p.y / FIELD_SQUARE_SIZE) % 2 == 0;
}

/* Group of functions that draw one piece of their item at position p. */

static void set_field_piece(pos p) {
    set_piece(p, is_dark_square(p) ? FIELD_DARK_COLOR : FIELD_BRIGHT_COLOR);
}

static void set_snake_piece(pos p) {
    set_piece(p, SNAKE_COLOR);
}

static void set_food_piece(pos p) {
    set_piece(p, FOOD_COLOR);
}

static void set_glossy_piece(pos p) {
    set_piece(p, GLOSSY_COLOR);
}

static void set_wall_piece(pos p) {
    set_piece(p, WALL_COLOR);
}

/* Returns the position one step ahead of the snake's head, in direction
   way. */
static pos target_ahead(direction way) {
    pos target = snake[SNAKE_HEAD];

    switch (way) {
        case UP:    target.y--; break;
        case DOWN:  target.y++; break;
        case LEFT:  target.x--; break;
        case RIGHT: target.x++; break;
    }
    return target;
}

/* Places the wall beside the food, on the side facing the snake's head, so
   the snake must go around it; its tip lines up with the food's middle
   row. Nothing is drawn yet. */
static void plan_wall(void) {
    wall_direction = (food_corner.x >= snake[SNAKE_HEAD].x) ? LEFT : RIGHT;
    wall_tip.y = food_corner.y + (food_size - 1) / 2;
    wall_tip.x = (wall_direction == LEFT) ? food_corner.x - 1 - WALL_ARM_LENGTH
                                         : food_corner.x + food_size + WALL_ARM_LENGTH;
}

/* TRUE if position p is one of the wall's pieces. The arms are diagonals: a
   piece h columns from the tip, toward the food, is h rows above or below
   it. */
static bool is_wall_piece(pos p) {
    int height = (wall_direction == LEFT) ? p.x - wall_tip.x : wall_tip.x - p.x;
    int half_base = abs(p.y - wall_tip.y);

    return height >= 0 && height <= WALL_ARM_LENGTH && half_base == height;
}

/* Group of functions that draw every piece of their item with set. */

static void fill_food(generic_set set) {
    pos p;

    for (p.y = food_corner.y; p.y < food_corner.y + food_size; p.y++)
        for (p.x = food_corner.x; p.x < food_corner.x + food_size; p.x++)
            set(p);
}

static void fill_wall(generic_set set) {
    pos upper, lower;
    int height;

    for (height = 0; height <= WALL_ARM_LENGTH; height++) {
        upper.x = (wall_direction == LEFT) ? wall_tip.x + height : wall_tip.x - height;
        upper.y = wall_tip.y - height;
        lower.x = upper.x;
        lower.y = wall_tip.y + height;
        set(upper);
        set(lower);
    }
}

/* Group of functions that return the area their items cover. */

static area food_area(void) {
    area a;

    a.top_left = food_corner;
    a.bottom_right.x = food_corner.x + food_size - 1;
    a.bottom_right.y = food_corner.y + food_size - 1;
    return a;
}

static area food_and_wall_area(void) {
    area a;

    a.top_left.x     = (wall_direction == LEFT) ? wall_tip.x : food_corner.x;
    a.top_left.y     = wall_tip.y - WALL_ARM_LENGTH;
    a.bottom_right.x = (wall_direction == LEFT) ? food_corner.x + food_size - 1 : wall_tip.x;
    a.bottom_right.y = wall_tip.y + WALL_ARM_LENGTH;
    return a;
}

/* TRUE if position p is inside the border, not on it. */
static bool is_inside_border(pos p) {
    return p.x > 0 && p.x < LAYOUT_WIDTH - 1 &&
           p.y > 0 && p.y < LAYOUT_HEIGHT - 1;
}

/* TRUE if area a is inside the border; for a rectangle, checking its two
   corners is enough. */
static bool is_area_inside_border(area a) {
    return is_inside_border(a.top_left) && is_inside_border(a.bottom_right);
}

/* TRUE if position p is in area a, edges included. */
static bool is_in_area(area a, pos p) {
    return p.x >= a.top_left.x && p.x <= a.bottom_right.x &&
           p.y >= a.top_left.y && p.y <= a.bottom_right.y;
}

/* TRUE if any of the snake's pieces is in area a. */
static bool is_snake_in_area(area a) {
    int i;

    for (i = SNAKE_TAIL; i <= SNAKE_HEAD; i++)
        if (is_in_area(a, snake[i]))
            return TRUE;
    return FALSE;
}

/* TRUE if position p is at least SAFE_DISTANCE steps from the wall's tip,
   counting the steps across plus the steps down. */
static bool is_far_from_wall(pos p) {
    return abs(wall_tip.x - p.x) + abs(wall_tip.y - p.y) >= SAFE_DISTANCE;
}

/* TRUE if the planned food and wall are a fair spot: far enough from the
   head for the player to react, inside the border, and clear of the
   snake. */
static bool is_spot_safe(void) {
    area footprint = food_and_wall_area();

    return is_far_from_wall(snake[SNAKE_HEAD]) &&
           is_area_inside_border(footprint) &&
           !is_snake_in_area(footprint);
}

/* Draws the field over everything inside the border. */
static void set_field(void) {
    pos p;

    for (p.y = 1; p.y < LAYOUT_HEIGHT - 1; p.y++)
        for (p.x = 1; p.x < LAYOUT_WIDTH - 1; p.x++)
            set_field_piece(p);
}

/* Draws the border: wall pieces all around the screen's edge. */
static void set_border(void) {
    pos p;

    for (p.x = 0; p.x < LAYOUT_WIDTH; p.x++) {
        p.y = 0;
        set_wall_piece(p);
        p.y = LAYOUT_HEIGHT - 1;
        set_wall_piece(p);
    }
    for (p.y = 1; p.y < LAYOUT_HEIGHT - 1; p.y++) {
        p.x = 0;
        set_wall_piece(p);
        p.x = LAYOUT_WIDTH - 1;
        set_wall_piece(p);
    }
}

/* Writes text on the HUD line, starting at x. */
static void set_hud_text(int x, const char *text) {
    pos p;

    p.y = HUD_Y;
    p.x = x;
    set_text(p, text, HUD_BACKGROUND_COLOR, HUD_TEXT_COLOR);
}

/* Writes value as two digits on the HUD line, at x. value must be 0 to
   99. */
static void set_hud_number(int x, int value) {
    char digits[3];

    digits[0] = '0' + value / 10;
    digits[1] = '0' + value % 10;
    digits[2] = '\0';
    set_hud_text(x, digits);
}

/* Writes the HUD's labels; the numbers go right after them. */
static void set_hud_labels(void) {
    set_hud_text(HUD_SECONDS_X, HUD_SECONDS_LABEL);
    set_hud_text(HUD_LEVEL_X, HUD_LEVEL_LABEL);
}

/* The x that centers something text_width positions wide on the screen. */
static int left_margin(int text_width) {
    return (LAYOUT_WIDTH - text_width) / 2;
}

/* Places the snake at its start, lying to the right of its tail, and draws
   it. */
static void set_snake(void) {
    int i;

    for (i = SNAKE_TAIL; i <= SNAKE_HEAD; i++) {
        snake[i].y = SNAKE_START_Y;
        snake[i].x = SNAKE_START_TAIL_X + i;
        set_snake_piece(snake[i]);
    }
}

/* Draws the start of a game: the field, the border, the HUD labels and the
   snake. Also seeds the random numbers that place the food. */
void set_layout(void) {
    srand(time(NULL));
    set_field();
    set_border();
    set_hud_labels();
    set_snake();
}

/* Covers the field and shows the game-over screen: the title in big
   letters and the prompt under it, both centered. */
void set_game_over(void) {
    pos p;

    set_field();
    p.y = GAME_OVER_TITLE_Y;
    p.x = left_margin(big_text_width(GAME_OVER_TITLE));
    set_big_text(p, GAME_OVER_TITLE, TITLE_COLOR);
    p.y = GAME_OVER_PROMPT_Y;
    p.x = left_margin(strlen(GAME_OVER_PROMPT));
    set_text(p, GAME_OVER_PROMPT, PROMPT_BACKGROUND_COLOR, PROMPT_TEXT_COLOR);
}

/* Group of functions that show their number in the HUD. */

void set_hud_level(int level) {
    set_hud_number(HUD_LEVEL_X + strlen(HUD_LEVEL_LABEL), level);
}

void set_hud_seconds(int seconds) {
    set_hud_number(HUD_SECONDS_X + strlen(HUD_SECONDS_LABEL), seconds);
}

/* Group of functions that tell whether the snake lies horizontally or
   vertically, judged by its tail and head alone. */

bool is_snake_horizontal(void) {
    return snake[SNAKE_TAIL].y == snake[SNAKE_HEAD].y;
}

bool is_snake_vertical(void) {
    return snake[SNAKE_TAIL].x == snake[SNAKE_HEAD].x;
}

/* Moves the snake one step in direction way: the tail piece becomes field
   again, and a new head piece is drawn in front. */
void move_snake(direction way) {
    int i;

    set_field_piece(snake[SNAKE_TAIL]);

    /* Each piece takes the place of the one ahead of it. */
    for (i = SNAKE_TAIL; i < SNAKE_HEAD; i++)
        snake[i] = snake[i + 1];

    snake[SNAKE_HEAD] = target_ahead(way);
    set_snake_piece(snake[SNAKE_HEAD]);
}

/* Places new food of side new_food_size, and its wall, at a random safe
   spot, and draws them. */
void set_food_and_wall(int new_food_size) {
    food_size = new_food_size;

    /* The corner is picked so the food itself stays inside the border;
       is_spot_safe checks the rest. */
    do {
        food_corner.x = 1 + rand() % (LAYOUT_WIDTH - 1 - food_size);
        food_corner.y = 1 + rand() % (LAYOUT_HEIGHT - 1 - food_size);
        plan_wall();
    } while (!is_spot_safe());

    fill_food(set_food_piece);
    fill_wall(set_wall_piece);
}

/* Draws the food in its glossy color, as it is eaten. */
void set_glossy_food(void) {
    fill_food(set_glossy_piece);
}

/* Draws field back over the food and the wall. */
void clear_food_and_wall(void) {
    fill_food(set_field_piece);
    fill_wall(set_field_piece);
}

/* Group of functions that tell whether the snake's next step, in direction
   way, lands on their item; the wall's includes the border. */

bool is_wall_ahead(direction way) {
    pos target = target_ahead(way);

    return !is_inside_border(target) || is_wall_piece(target);
}

bool is_food_ahead(direction way) {
    return is_in_area(food_area(), target_ahead(way));
}
