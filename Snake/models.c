#include <stdlib.h>

#include "models.h"

Snake make_snake() {
	Snake snake;
	snake.lines[0] = (Point) { 3, 3 };
	snake.lines[1] = (Point) { 3, 7 };
	snake.lines[1] = (Point) { 7, 7 };
	snake.length = 3;
	return snake;
}

Apple make_apple() {
	Apple apple;
	apple.pos = (Point) { 25, 25 };
	return apple;
}