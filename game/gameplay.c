#include "game\gameplay.h"
#include "game\layout.h"
#include "game\controls.h"
#include "game\melodies.h"
#include "platform\clock.h"
#include "platform\sound.h"

/* What changes from level to level: the snake's speed, in steps a second;
   the time to reach the food, in seconds; and the food's side, in pieces. */
typedef struct {
    int steps_per_second;
    int time_limit;
    int food_size;
} level_rules;

/* Where the game waits: ACTIVE while the snake moves, IDLE on the
   game-over screen. */
typedef enum { ACTIVE, IDLE } game_state;

/* What happened, as next_event reports it: a key's command, a second
   passing or the last one running out, or what lies ahead of the snake
   when it is time to step. */
typedef enum {
    NO_EVENT,
    DIRECTION_CHOSEN,
    RESTART_CHOSEN,
    QUIT_CHOSEN,
    SECOND_PASSED,
    TIME_OUT,
    WALL_AHEAD,
    FOOD_AHEAD,
    FIELD_AHEAD
} event;

static game_state state = ACTIVE;
static bool running = TRUE;

static int level_number;
static level_rules level;
static direction snake_direction;
static int seconds_left;
static interval_timer step_timer;
static interval_timer second_timer;

/* The direction of the latest DIRECTION_CHOSEN event: next_event sets it,
   and update_game hands it to turn. */
static direction chosen_direction;

static void apply_level_difficulty(void);
static void set_level(int number);
static void set_hud(void);
static void set_timers(void);
static bool is_direction_horizontal(direction way);
static bool is_direction_vertical(direction way);
static event next_event(void);
static void turn(direction new_direction);
static void lose_second(void);
static void step(void);
static void eat(void);
static void die(void);
static void quit(void);

/* Changes the rules for the level that has just started. Level 1 sets them
   all; a few later levels change some of them, and every other level keeps
   the rules of the one before. */
static void apply_level_difficulty(void) {
    if      (level_number == 1)  { level.steps_per_second = 3;  level.time_limit = 60; level.food_size = 3; }
    else if (level_number == 3)  { level.steps_per_second = 4;  level.time_limit = 45; }
    else if (level_number == 5)  { level.food_size = 2; }
    else if (level_number == 7)  { level.steps_per_second = 5; }
    else if (level_number == 8)  { level.time_limit = 30; }
    else if (level_number == 10) { level.food_size = 1; }
    else if (level_number == 12) { level.steps_per_second = 7; }
    else if (level_number == 15) { level.steps_per_second = 10; level.time_limit = 20; }

    /* Every level starts with its full time limit. */
    seconds_left = level.time_limit;
}

/* Starts level number, with its rules and its full time limit. Levels must
   start in order, 1, 2, 3 and on, since each one changes only what
   differs from the one before. */
static void set_level(int number) {
    level_number = number;
    apply_level_difficulty();
}

/* Shows the level and the seconds left in the HUD. */
static void set_hud(void) {
    set_hud_level(level_number);
    set_hud_seconds(seconds_left);
}

/* Starts the step timer at the level's speed, and the second timer, both
   counting from now. */
static void set_timers(void) {
    start_interval_timer(&step_timer, level.steps_per_second);
    start_interval_timer(&second_timer, 1);
}

/* Group of functions that tell whether direction way is horizontal or
   vertical. */

static bool is_direction_horizontal(direction way) {
    return way == LEFT || way == RIGHT;
}

static bool is_direction_vertical(direction way) {
    return way == UP || way == DOWN;
}

/* Returns one thing that happened since the last call, checking a key
   first, then the second timer, then the step timer; anything else waits
   for the next call. NO_EVENT if nothing happened. */
static event next_event(void) {
    switch (read_command()) {
        case GO_UP:    chosen_direction = UP;    return DIRECTION_CHOSEN;
        case GO_DOWN:  chosen_direction = DOWN;  return DIRECTION_CHOSEN;
        case GO_LEFT:  chosen_direction = LEFT;  return DIRECTION_CHOSEN;
        case GO_RIGHT: chosen_direction = RIGHT; return DIRECTION_CHOSEN;
        case RESTART:  return RESTART_CHOSEN;
        case QUIT:     return QUIT_CHOSEN;
        default:       break;
    }

    /* When the last second passes, it is a time-out. */
    if (has_interval_passed(&second_timer))
        return (seconds_left > 1) ? SECOND_PASSED : TIME_OUT;

    /* When the snake is due to step, the event says what it would step
       into. */
    if (has_interval_passed(&step_timer)) {
        if (is_wall_ahead(snake_direction)) return WALL_AHEAD;
        if (is_food_ahead(snake_direction)) return FOOD_AHEAD;
        return FIELD_AHEAD;
    }

    return NO_EVENT;
}

/* Turns the snake to new_direction, but only across its body: a horizontal
   snake can turn up or down, a vertical one left or right. */
static void turn(direction new_direction) {
    if (is_snake_horizontal() && is_direction_vertical(new_direction))
        snake_direction = new_direction;
    else if (is_snake_vertical() && is_direction_horizontal(new_direction))
        snake_direction = new_direction;
}

/* Counts one second down, on the HUD too. */
static void lose_second(void) {
    seconds_left--;
    set_hud_seconds(seconds_left);
}

/* Moves the snake one step in its direction. */
static void step(void) {
    move_snake(snake_direction);
}

/* The snake eats the food: the food flashes during the new-level melody,
   then the snake steps in and the next level starts, with new food and
   wall and fresh timers, so the melody's time isn't made up in quick
   steps. */
static void eat(void) {
    set_glossy_food();
    play_melody(NEW_LEVEL_MELODY);
    clear_food_and_wall();
    step();
    set_level(level_number + 1);
    set_hud();
    set_food_and_wall(level.food_size);
    set_timers();
}

/* Shows the game-over screen and plays the game-over melody. */
static void die(void) {
    set_game_over();
    play_melody(GAME_OVER_MELODY);
}

/* Ends the game: main's loop stops at its next check. */
static void quit(void) {
    running = FALSE;
}

/* Starts a new game at level 1: draws the layout, the HUD, the food and
   wall, starts the timers, and sends the snake right. */
void start_game(void) {
    set_layout();
    set_level(1);
    set_hud();
    set_food_and_wall(level.food_size);
    set_timers();
    snake_direction = RIGHT;
}

/* FALSE once the player has quit. */
bool is_game_running(void) {
    return running;
}

/* Handles the next event by the current state: does the event's actions
   and moves to the next state. main calls it over and over. */
void update_game(void) {
    event trigger = next_event();

    switch (state) {
        case ACTIVE:
            switch (trigger) {
                case DIRECTION_CHOSEN: turn(chosen_direction); state = ACTIVE; break;
                case SECOND_PASSED:    lose_second();          state = ACTIVE; break;
                case TIME_OUT:         lose_second(); die();   state = IDLE;   break;
                case WALL_AHEAD:       die();                  state = IDLE;   break;
                case FOOD_AHEAD:       eat();                  state = ACTIVE; break;
                case FIELD_AHEAD:      step();                 state = ACTIVE; break;
                case NO_EVENT:                                 state = ACTIVE; break;
                default:                                       state = ACTIVE; break;
            }
            break;

        case IDLE:
            switch (trigger) {
                case QUIT_CHOSEN:      quit();                 state = IDLE;   break;
                case RESTART_CHOSEN:   start_game();           state = ACTIVE; break;
                case NO_EVENT:                                 state = IDLE;   break;
                default:                                       state = IDLE;   break;
            }
            break;
    }
}
