// http://gafferongames.com/game-physics/fix-your-timestep/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "game.h"
#include "util.h"

GLuint vao;
GLuint ebo;
GLuint vbo;
GLenum glErr;

GLuint snakeShaderProgram;
GLuint appleShaderProgram;

void opengl_setup() {
	// Create Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a Vertex Buffer Object
    glGenBuffers(1, &vbo);

	// Create an element array
	glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

	char* shaderSrc;

	shaderSrc = read_file("snake.vertexshader");
	GLuint snakeVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(snakeVertexShader, 1, &shaderSrc, NULL);
	glCompileShader(snakeVertexShader);
	free(shaderSrc);

	shaderSrc = read_file("snake.fragmentshader");
	GLuint snakeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(snakeFragmentShader, 1, &shaderSrc, NULL);
	glCompileShader(snakeFragmentShader);
	free(shaderSrc);

	// Link the vertex and fragment shader into a shader program
	snakeShaderProgram = glCreateProgram();
	glAttachShader(snakeShaderProgram, snakeVertexShader);
	glAttachShader(snakeShaderProgram, snakeFragmentShader);
	glBindFragDataLocation(snakeShaderProgram, 0, "outColor");
	glLinkProgram(snakeShaderProgram);

	shaderSrc = read_file("apple.vertexshader");
	GLuint appleVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(appleVertexShader, 1, &shaderSrc, NULL);
	glCompileShader(appleVertexShader);
	free(shaderSrc);

	shaderSrc = read_file("apple.fragmentshader");
	GLuint appleFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(appleFragmentShader, 1, &shaderSrc, NULL);
	glCompileShader(appleFragmentShader);
	free(shaderSrc);

	// Link the vertex and fragment shader into a shader program
	appleShaderProgram = glCreateProgram();
	glAttachShader(appleShaderProgram, appleVertexShader);
	glAttachShader(appleShaderProgram, appleFragmentShader);
	glBindFragDataLocation(appleShaderProgram, 0, "outColor");
	glLinkProgram(appleShaderProgram);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(snakeShaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE,
		2 * sizeof(float), 0);
}

Point get_cell_size() {
	static Point size = { 0, 0 };
	if (size.x == 0 && size.y == 0) {
		size.x = scale(0, BOARD_WIDTH, 0, 2, 1);
		size.y = scale(0, BOARD_HEIGHT, 0, 2, 1);
	}
	return size;
}

Point scale_screen(Point p) {
	return (Point) {
		scale(0, BOARD_WIDTH, -1, 1, p.x),
		scale(0, BOARD_HEIGHT, -1, 1, p.y)
	};
}

void update_state(GameState* state, const double t, const double dt) {
	static float rate = 5.0f;
	static float cycleT = 0;
	static unsigned int snakeTailGrows = 0;
	static unsigned int snakeTailGrowRate = 15;
	Direction dir = state->snake.direction;
	Snake* snake = &state->snake;

	cycleT += (float)dt;

	if (glfwGetKey(state->window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (dir == LEFT || dir == RIGHT) {
			snake->nextDirection = UP;
		}
	} else if (glfwGetKey(state->window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (dir == LEFT || dir == RIGHT) {
			snake->nextDirection = DOWN;
		}
	} else if (glfwGetKey(state->window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (dir == DOWN || dir == UP) {
			snake->nextDirection = LEFT;
		}
	} else if (glfwGetKey(state->window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (dir == DOWN || dir == UP) {
			snake->nextDirection = RIGHT;
		}
	}

	if (cycleT * rate >= 1) {
		cycleT = 0;
		if (snakeTailGrows > 0) {
			snakeTailGrows -= 1;
		}
		if (snake->nextDirection != snake->direction) {
			Point p = snake_get_point(*snake, 0);
			p.x = roundf(p.x);
			p.y = roundf(p.y);
			snake_set_point(snake, 0, p);
			snake_unshift_point(snake, p);
			snake->direction = snake->nextDirection;
		}
	}

	Point head, tail1, tail2;
	head = snake_get_point(*snake, 0);
	float distance = (float)dt * rate;
	if (dir == UP) {
		head.y += distance;
	} else if (dir == DOWN) {
		head.y -= distance;
	} else if (dir == LEFT) {
		head.x -= distance;
	} else if (dir == RIGHT) {
		head.x += distance;
	}
	if (cycleT == 0) {
		head = rounded_point(head);
	}
	snake_set_point(snake, 0, head);

	if (cycleT == 0 && points_eq(snake_get_point(*snake, 0), state->apple.pos)) {
		state->apple = make_apple();
		snakeTailGrows += snakeTailGrowRate;
	}

	if (snakeTailGrows == 0) {
		tail1 = snake_rget_point(*snake, 0);
		tail2 = rounded_point(snake_rget_point(*snake, 1));
		if (tail1.x != tail2.x) {
			tail1.x += tail1.x < tail2.x ? distance : -distance;
		} else if (tail1.y != tail2.y) {
			tail1.y += tail1.y < tail2.y ? distance : -distance;
		}
		if (cycleT == 0) {
			tail1 = rounded_point(tail1);
		}
		if (cycleT == 0 && points_eq(tail1, tail2)) {
			snake_pop_point(snake);
		} else {
			snake_rset_point(snake, 0, tail1);
		}
	}
}

void draw_rectangle(Point a, Point b) {
	static float w = 0;
	static float h = 0;
	if (w == 0) {
		w = scale(0, BOARD_WIDTH, 0, 2, 1);
		h = scale(0, BOARD_HEIGHT, 0, 2, 1);
	}
	
	Point a1 = scale_screen(a);
	Point a2 = (Point) { a1.x + w, a1.y + h };
	Point b1 = scale_screen(b);
	Point b2 = (Point) { b1.x + w, b1.y + h };

	GLuint elements[6] = { 0, 1, 2, 2, 3, 0 };
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STREAM_DRAW);

	float vertices[2 * 4] = {
		minf(a1.x, b1.x), minf(a1.y, b1.y),
		maxf(a2.x, b2.x), minf(a1.y, b1.y),
		maxf(a2.x, b2.x), maxf(a2.y, b2.y),
		minf(a1.x, b1.x), maxf(a2.y, b2.y)
	};
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void render_snake(Snake snake) {
	glUseProgram(snakeShaderProgram);
	float snake_w = scale(0, BOARD_WIDTH, 0, 2, 1);
	float snake_h = scale(0, BOARD_HEIGHT, 0, 2, 1);
	for (unsigned int i = 1; i < snake.length; i++) {
		draw_rectangle(snake_get_point(snake, i - 1), snake_get_point(snake, i));
	}
}

void render_apple(Apple apple) {
	glUseProgram(appleShaderProgram);
	draw_rectangle(apple.pos, apple.pos);
}

void render_frame(GameState state) {
	// Clear the screen to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	render_snake(state.snake);
	render_apple(state.apple);

	glfwSwapBuffers(state.window);
}

int game_loop(GameState state) {
	double t = 0.0;
	const double dt = 0.01;

	double currentTime = time_since_start();
	double accumulator = 0.0;

	bool quit = false;
	while (!quit) {
		glfwPollEvents();
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
	return result;
}
