#include <stdlib.h>

#include "models.h"

Snake make_snake() {
	Snake snake;
	snake.lines[0] = (Point) { 3, 7 };
	snake.lines[1] = (Point) { 3, 3 };
	snake.start = 0;
	snake.length = 2;
	snake.direction = UP;
	return snake;
}

Point snake_get_point(Snake snake, unsigned int i) {
	i += snake.start;
	if (i >= SNAKE_MAX_POINTS) {
		i -= SNAKE_MAX_POINTS;
	}
	return snake.lines[i];
}

Point snake_rget_point(Snake snake, unsigned int i) {
	return snake_get_point(snake, snake.length - 1 - i);
}

void snake_set_point(Snake* snake, unsigned int i, Point p) {
	i += snake->start;
	if (i >= SNAKE_MAX_POINTS) {
		i -= SNAKE_MAX_POINTS;
	}
	snake->lines[i] = p;
}

void snake_rset_point(Snake* snake, unsigned int i, Point p) {
	return snake_set_point(snake, snake->length - 1 - i, p);
}

void snake_push_point(Snake* snake, Point p) {
	unsigned int i = snake->start + snake->length;
	if (i >= SNAKE_MAX_POINTS) {
		i -= SNAKE_MAX_POINTS;
	}
	snake->lines[i] = p;
	snake->length += 1;
}

void snake_unshift_point(Snake* snake, Point p) {
	if (snake->start == 0) {
		snake->start = SNAKE_MAX_POINTS - 1;
	} else {
		snake->start -= 1;
	}
	snake->lines[snake->start] = p;
	snake->length += 1;
}

Point snake_shift_point(Snake* snake) {
	Point p = snake_get_point(*snake, 0);
	snake->start += 1;
	if (snake->start >= SNAKE_MAX_POINTS) {
		snake->start -= SNAKE_MAX_POINTS;
	}
	snake->length -= 1;
	return p;
}

Point snake_pop_point(Snake* snake) {
	Point p = snake_get_point(*snake, snake->length - 1);
	snake->length -= 1;
	return p;
}

Apple make_apple() {
	Apple apple;
	apple.pos = (Point) { 25, 25 };
	return apple;
}