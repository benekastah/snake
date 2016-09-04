#pragma once

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/GL.h>
#include <GLFW/glfw3.h>

#include "models.h"

typedef struct {
	Snake snake;
	Apple apple;
	GLFWwindow* window;
} GameState;
