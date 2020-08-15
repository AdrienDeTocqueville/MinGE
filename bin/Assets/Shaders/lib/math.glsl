#ifndef MATH_GLSL
#define MATH_GLSL

const float PI = 3.14159265359;

float pow5(float x)
{
    float x2 = x * x;
    return x2 * x2 * x;
}

float saturate(float x)
{
	return clamp(x, 0.0, 1.0);
}

#endif
