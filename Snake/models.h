#pragma once

#include <stdbool.h>

#define BOARD_HEIGHT 100
#define BOARD_WIDTH 100
#define SNAKE_MAX_POINTS (BOARD_HEIGHT*BOARD_WIDTH)

// MISC
typedef struct { float x; float y; } Point;

bool points_eq(Point a, Point b);
Point rand_point();
Point rounded_point(Point p);
Point scale_point(Point min1, Point max1, Point min2, Point max2, Point value);

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

// SNEK
typedef struct {
	Point lines[SNAKE_MAX_POINTS];
	Direction direction[3];
	bool alive;
	unsigned int length;
	unsigned int start;
	unsigned int growing;
	unsigned int points;
	float rate;
} Snake;

Snake make_snake();

Point snake_get_point(Snake snake, unsigned int i);
Point snake_rget_point(Snake snake, unsigned int i);
void snake_set_point(Snake* snake, unsigned int i, Point p);
void snake_rset_point(Snake* snake, unsigned int i, Point p);
Point snake_shift_point(Snake* snake);
Point snake_pop_point(Snake* snake);
void snake_push_point(Snake* snake, Point p);
void snake_unshift_point(Snake* snake, Point p);


// APPLE
typedef struct {
	Point pos;
} Apple;

Apple make_apple();