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

GLuint shaderProgram;
GLuint colorUniform;

GLuint create_shader_program(char* vertexShaderFile, char* fragmentShaderFile) {
    char* shaderSrc;

    shaderSrc = read_file(vertexShaderFile);
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &shaderSrc, NULL);
    glCompileShader(vertexShader);
    free(shaderSrc);

    shaderSrc = read_file(fragmentShaderFile);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &shaderSrc, NULL);
    glCompileShader(fragmentShader);
    free(shaderSrc);

    // Link the vertex and fragment shader into a shader program
    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glBindFragDataLocation(shader, 0, "outColor");
    glLinkProgram(shader);

    return shader;
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

    char* shaderSrc;

    shaderProgram = create_shader_program("snake.vertexshader", "snake.fragmentshader");
    colorUniform = glGetUniformLocation(shaderProgram, "color");
    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE,
        2 * sizeof(float), 0);
}

KeyState key_advance_state(KeyState ks) {
    if (ks == UNPRESSED) {
        return PRESSING;
    } else {
        return PRESSED;
    }
}

Point arena[2];
void set_arena(GLFWwindow* window, int width, int height) {
    if (width == 0 && height == 0) {
        glfwGetWindowSize(window, &width, &height);
    }
    const float pad = 60;
    float w = (float) width;
    float h = (float) height;
    arena[0] = (Point) {
        w > h ? (w - h) / 2 : pad,
        w > h ? pad : (h - w) / 2
    };
    arena[1] = (Point) {
        w > h ? w - (w - h) / 2 : w - pad,
        w > h ? h - pad : h - (h - w) / 2
    };
    arena[0] = scale_point((Point) { 0, 0 }, (Point) { w, h },
        (Point) { -1, -1 }, (Point) { 1, 1 }, arena[0]);
    arena[1] = scale_point((Point) { 0, 0 }, (Point) { w, h },
        (Point) { -1, -1 }, (Point) { 1, 1 }, arena[1]);
}

Point scale_screen(Point p) {
    return (Point) {
        scale(0, BOARD_WIDTH, arena[0].x, arena[1].x, p.x),
        scale(0, BOARD_HEIGHT, arena[0].y, arena[1].y, p.y)
    };
}

bool detect_collision(Snake snake) {
    Point head = snake_get_point(snake, 0);

    // Out of bounds check
    if (head.x < 0 || head.y < 0 || head.x > BOARD_WIDTH || head.y > BOARD_HEIGHT) {
        return true;
    }

    // Body collision check
    Point a, b;
    for (unsigned int i = 1; i < snake.length; i++) {
        a = snake_get_point(snake, i - 1);
        b = snake_get_point(snake, i);
        if (points_eq(head, a) || points_eq(head, b)) {
            continue;
        }
        if (a.x == b.x && head.x == a.x) {
            if (head.y >= minf(a.y, b.y) && head.y <= maxf(a.y, b.y)) {
                return true;
            }
        } else if (a.y == b.y && head.y == a.y) {
            if (head.x >= minf(a.x, b.x) && head.x <= maxf(a.x, b.x)) {
                return true;
            }
        }
    }

    return false;
}

void update_state(GameState* state, const double t, const double dt) {
    static float cycleT = 0;
    Snake* snake = &state->snake;

    cycleT += (float)dt;
    if (cycleT * snake->rate >= 1) {
        cycleT = 0;
    }

    state->keys.up = glfwGetKey(state->window, GLFW_KEY_UP) == GLFW_PRESS ?
        key_advance_state(state->keys.up) : UNPRESSED;
    state->keys.down = glfwGetKey(state->window, GLFW_KEY_DOWN) == GLFW_PRESS ?
        key_advance_state(state->keys.down) : UNPRESSED;
    state->keys.left = glfwGetKey(state->window, GLFW_KEY_LEFT) == GLFW_PRESS ?
        key_advance_state(state->keys.left) : UNPRESSED;
    state->keys.right = glfwGetKey(state->window, GLFW_KEY_RIGHT) == GLFW_PRESS ?
        key_advance_state(state->keys.right) : UNPRESSED;

    for (int i = 1; i < 3; i++) {
        Direction dir1 = snake->direction[i - 1];
        Direction dir2 = snake->direction[i];
        if (dir1 == LEFT || dir1 == RIGHT) {
            if (state->keys.up == PRESSING) {
                snake->direction[i] = UP;
            } else if (state->keys.down == PRESSING) {
                snake->direction[i] = DOWN;
            }
        } else {
            if (state->keys.left == PRESSING) {
                snake->direction[i] = LEFT;
            } else if (state->keys.right == PRESSING) {
                snake->direction[i] = RIGHT;
            }
        }
        if (i == 1 && dir2 != snake->direction[i]) {
            snake->direction[i + 1] = snake->direction[i];
        }
    }

    if (cycleT == 0) {
        if (snake->growing > 0) {
            snake->growing -= 1;
        }
        if (snake->direction[0] != snake->direction[1]) {
            Point p = snake_get_point(*snake, 0);
            p.x = roundf(p.x);
            p.y = roundf(p.y);
            snake_set_point(snake, 0, p);
            snake_unshift_point(snake, p);
            snake->direction[0] = snake->direction[1];
            snake->direction[1] = snake->direction[2];
        }
    }

    Point head, tail1, tail2;
    head = snake_get_point(*snake, 0);
    float distance = (float)dt * snake->rate;
    Direction dir = snake->direction[0];
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
        snake->points += 1;
        snake->growing += 5 + snake->points;
        snake->rate *= 1.1f;
    }

    if (snake->growing == 0) {
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

    if (detect_collision(*snake)) {
        state->snake = make_snake();
    }
}

void draw_rectangle(Point a, Point b) {
    Point halfCell = (Point) { 0.5f, 0.5f };
    Point a1 = scale_screen(subtract_points(a, halfCell));
    Point a2 = scale_screen(add_points(a, halfCell));
    Point b1 = scale_screen(subtract_points(b, halfCell));
    Point b2 = scale_screen(add_points(b, halfCell));

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
    glUniform3f(colorUniform, 1.0f, 1.0f, 0.0f);
    float snake_w = scale(0, BOARD_WIDTH, 0, 2, 1);
    float snake_h = scale(0, BOARD_HEIGHT, 0, 2, 1);
    for (unsigned int i = 1; i < snake.length; i++) {
        draw_rectangle(snake_get_point(snake, i - 1), snake_get_point(snake, i));
    }
}

void render_apple(Apple apple) {
    glUniform3f(colorUniform, 1.0f, 0.0f, 0.0f);
    draw_rectangle(apple.pos, apple.pos);
}

void render_arena() {
    glUniform3f(colorUniform, 0.2f, 0.2f, 0.2f);
    draw_rectangle((Point) { 0, 0 }, (Point) { BOARD_WIDTH, BOARD_HEIGHT });
}

void render_frame(GameState state) {
    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    render_arena();
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
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Snake", monitor, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return window;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    set_arena(window, 0, 0);
    glfwSetWindowSizeCallback(window, set_arena);

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
    state.keys = (Keys) { UNPRESSED, UNPRESSED, UNPRESSED, UNPRESSED };
    state.window = window;

    opengl_setup();
    
    int result = game_loop(state);

    glfwTerminate();
    return result;
}
