#pragma once

#include <stdio.h>
#include <stdlib.h>

char* read_file(const char* fname);

float scale(float min1, float max1, float min2, float max2, float val);
float randf_btwn(float min, float max);

float minf(float a, float b);
float maxf(float a, float b);

void print_gl_errors();