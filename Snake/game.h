#pragma once

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/GL.h>
#include <GLFW/glfw3.h>

#include "models.h"

typedef enum { UNPRESSED, PRESSING, PRESSED } KeyState;

KeyState key_advance_state(KeyState ks);

typedef struct {
    KeyState up, down, left, right;
} Keys;

typedef struct {
    Snake snake;
    Apple apple;
    Keys keys;
    GLFWwindow* window;
} GameState;
