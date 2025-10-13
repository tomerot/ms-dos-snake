#ifndef LAYOUT_H
#define LAYOUT_H

#include "common.h"

typedef enum { UP, DOWN, LEFT, RIGHT } direction;

void set_layout(void);
void set_game_over(void);

void set_hud_level(int level);
void set_hud_seconds(int seconds);

void set_food_and_wall(int new_food_size);
void set_glossy_food(void);
void clear_food_and_wall(void);

bool is_snake_horizontal(void);
bool is_snake_vertical(void);
bool is_wall_ahead(direction way);
bool is_food_ahead(direction way);

void move_snake(direction way);

#endif
