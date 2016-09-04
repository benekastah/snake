#pragma once

#define BOARD_HEIGHT 100
#define BOARD_WIDTH 100
#define SNAKE_MAX_POINTS (BOARD_HEIGHT*BOARD_WIDTH)

// MISC
typedef struct { int x; int y; } Point;
typedef struct { float x; float y; } Pointf;

// SNEK
typedef struct {
	Point lines[SNAKE_MAX_POINTS];
	int length;
} Snake;

Snake make_snake();


// APPLE
typedef struct {
	Point pos;
} Apple;

Apple make_apple();