#include <stdlib.h>
#include <stdarg.h>

#include <windows.h>

#include <GL/glew.h>
#include <GL/GL.h>

#include "util.h"

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

char* read_file(const char* fname) {
	char* buffer = NULL;
	long length;
	FILE* f;
	errno_t err = fopen_s(&f, fname, "rb");

	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = calloc(1, length + 1);
		if (buffer) {
			fread(buffer, 1, length, f);
		}
		fclose(f);
	}

	return buffer;
}

float minf(float a, float b) {
	return a < b ? a : b;
}

float maxf(float a, float b) {
	return a > b ? a : b;
}

float scale(float min1, float max1, float min2, float max2, float val) {
	float d1 = max1 - min1;
	float d2 = max2 - min2;
	float ratio = d2 / d1;
	return ((val - min1) * ratio) + min2;
}

float randf_btwn(float min, float max) {
	return scale(0, RAND_MAX, min, max, (float)rand());
}

void print_gl_errors() {
	GLenum glErr;
	while ((glErr = glGetError()) != GL_NO_ERROR) {
		if (glErr == GL_INVALID_ENUM) {
			printf("GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag\n");
		}
		if (glErr == GL_INVALID_VALUE) {
			printf("GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag\n");
		}
		if (glErr == GL_INVALID_OPERATION) {
			printf("GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag\n");
		}
		if (glErr == GL_INVALID_FRAMEBUFFER_OPERATION) {
			printf("GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag\n");
		}
		if (glErr == GL_OUT_OF_MEMORY) {
			printf("GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded\n");
		}
		if (glErr == GL_STACK_UNDERFLOW) {
			printf("GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow\n");
		}
		if (glErr == GL_STACK_OVERFLOW) {
			printf("GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow\n");
		}
	}
}
