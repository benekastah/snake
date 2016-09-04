// http://gafferongames.com/game-physics/fix-your-timestep/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <windows.h>

#include "game.h"
#include "util.h"

GLuint vao;
GLuint ebo;
GLuint vbo;
GLenum glErr;

float* snakeVertices;
GLuint* snakeElements;

double time_since_start() {
	static int initialized = 0;
	static LARGE_INTEGER start, freq;
	LARGE_INTEGER now;
	if (!initialized) {
		initialized = 1;
		QueryPerformanceCounter(&start);
		QueryPerformanceFrequency(&freq);
	}
	QueryPerformanceCounter(&now);
	return (double)(now.QuadPart - start.QuadPart) / freq.QuadPart;
}

double pulse(double t, double f, double min, double max) {
	static const float pi = 3.14f;
	double result = 0.5f * sin(2 * pi * f * t);
	result -= min;
	result *= max - min;
	return result;
}

float scale(float min1, float max1, float min2, float max2, float val) {
	float d1 = max1 - min1;
	float d2 = max2 - min2;
	float ratio = d2 / d1;
	return ((val - min1) * ratio) + min2;
}

Pointf scale_screen(Point p) {
	return (Pointf) {
		scale(0, (float)BOARD_WIDTH, -1, 1, (float)p.x),
		scale(0, (float)BOARD_HEIGHT, -1, 1, (float)p.y)
	};
}

void update_state(GameState* state, const double t, const double dt) {
}

void opengl_setup() {
	// Create Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a Vertex Buffer Object
    glGenBuffers(1, &vbo);

	// Create an element array
	glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

	char *vertexSrc = read_file("snake.vertexshader");
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSrc, NULL);
	glCompileShader(vertexShader);
	free(vertexSrc);

	char *fragmentSrc = read_file("snake.fragmentshader");
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
	glCompileShader(fragmentShader);
	free(fragmentSrc);

	// Link the vertex and fragment shader into a shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE,
		2 * sizeof(float), 0);
}

void render_frame(GameState state) {
	// Clear the screen to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	float snake_w = scale(0, BOARD_WIDTH, 0, 2, 1);
	float snake_h = scale(0, BOARD_HEIGHT, 0, 2, 1);

	int snakeVerticesLengthPerLine = 2 * 4;
	int snakeVerticesSize = sizeof(float) * snakeVerticesLengthPerLine * (state.snake.length - 1);
	snakeVertices = realloc(snakeVertices, snakeVerticesSize);
	int snakeElementsLengthPerLine = 6;
	int snakeElementsSize = sizeof(GLuint) * snakeElementsLengthPerLine * (state.snake.length - 1);
	snakeElements = realloc(snakeElements, snakeElementsSize);

	Pointf a1, a2, b1, b2;
	for (int i = 1; i < state.snake.length; i++) {
		a1 = scale_screen(state.snake.lines[i - 1]);
		a2 = (Pointf) { a1.x + snake_w, a1.y + snake_h };
		b1 = scale_screen(state.snake.lines[i]);
		b2 = (Pointf) { b1.x + snake_w, b1.y + snake_h };

		GLuint elements[] = { 0, 1, 2, 2, 3, 0 };
		memcpy(snakeElements + (i - 1) * snakeElementsLengthPerLine, &elements, sizeof(elements));

		float vertices[] = {
			minf(a1.x, b1.x), minf(a1.y, b1.y),
			maxf(a2.x, b2.x), minf(a1.y, b1.y),
			maxf(a2.x, b2.x), maxf(a2.y, b2.y),
			minf(a1.x, b1.x), maxf(a2.y, b2.y)
		};
		memcpy(snakeVertices + (i - 1) * snakeVerticesLengthPerLine, &vertices, sizeof(vertices));
	}

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, snakeVerticesSize, snakeVertices, GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, snakeElementsSize, snakeElements, GL_STREAM_DRAW);

	glDrawElements(GL_TRIANGLES, (state.snake.length - 1) * 6, GL_UNSIGNED_INT, 0);

	glfwSwapBuffers(state.window);
}

int game_loop(GameState state) {
	double t = 0.0;
	const double dt = 0.01;

	double currentTime = time_since_start();
	double accumulator = 0.0;

	bool quit = false;
	while (!quit) {
		double newTime = time_since_start();
		double frameTime = newTime - currentTime;
		currentTime = newTime;

		accumulator += frameTime;

		while (accumulator >= dt) {
			update_state(&state, t, dt);
			accumulator -= dt;
			t += dt;
		}

		render_frame(state);

		glfwPollEvents();
		quit = glfwGetKey(state.window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(state.window) != 0;
	}
	return 0;
}

GLFWwindow* getWindow() {
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Tutorial 01", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return window;
	}
	glfwMakeContextCurrent(window); // Initialize GLEW
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	return window;
}

int main(void) {
	snakeVertices = malloc(1);
	snakeElements = malloc(1);

	GLFWwindow* window = getWindow();
	if (window == NULL) {
		return -1;
	}

	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	print_gl_errors();  // Ignore GL_INVALID_ENUM

	GameState state;
	state.snake = make_snake();
	state.apple = make_apple();
	state.window = window;

	opengl_setup();
	
	int result = game_loop(state);

	glfwTerminate();
	free(snakeVertices);
	free(snakeElements);
	return result;
}
