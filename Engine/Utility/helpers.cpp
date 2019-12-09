#include "Utility/helpers.h"

vec2 toVec2(sf::Vector2i v)
{
	return vec2(v.x, v.y);
}
vec2 toVec2(sf::Vector2u v)
{
	return vec2(v.x, v.y);
}
sf::Vector2i toSFVec2i(vec2 v)
{
	return sf::Vector2i(v.x, v.y);
}
sf::Vector2u toSFVec2u(vec2 v)
{
	return sf::Vector2u(v.x, v.y);
}

vec3 vecClamp(vec3 v)
{
	for (unsigned i(0) ; i < 3 ; i++)
		if (epsilonEqual(v[i], 0.0f, EPSILON))
			v[i] = 0.0f;

	return v;
}

void write(const vec3 &x, bool ret)
{
	printf("(%f, %f, %f)", x[0], x[1], x[2]);
	if (ret) putchar('\n');
}

void write(const mat4 &x, bool ret)
{
	printf("((%f, %f, %f, %f)\n (%f, %f, %f, %f)\n (%f, %f, %f, %f)\n (%f, %f, %f, %f))",
		(x[0][0]), (x[0][1]), (x[0][2]), (x[0][3]),
		(x[1][0]), (x[1][1]), (x[1][2]), (x[1][3]),
		(x[2][0]), (x[2][1]), (x[2][2]), (x[2][3]),
		(x[3][0]), (x[3][1]), (x[3][2]), (x[3][3]));
	if (ret) putchar('\n');
}

bool epsilonEqual(const quat& a, const quat& b, float epsilon)
{
	for (unsigned i(0) ; i < 4 ; i++)
	if (!epsilonEqual(a[i], b[i], epsilon))
		return false;

	return true;
}

#ifndef NO_SIMD
#include <immintrin.h>
#endif // NO_SIMD

void simd_mul(const mat4& a, const mat4& b, mat4& out)
{
#ifndef NO_SIMD
	__m128 r0 = _mm_loadu_ps(&a[0][0]);
	__m128 r1 = _mm_loadu_ps(&a[1][0]);
	__m128 r2 = _mm_loadu_ps(&a[2][0]);
	__m128 r3 = _mm_loadu_ps(&a[3][0]);

	__m128 l = _mm_loadu_ps(&b[0][0]);
	__m128 t0 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(0,0,0,0)),r0);
	__m128 t1 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(1,1,1,1)),r1);
	__m128 t2 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(2,2,2,2)),r2);
	__m128 t3 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(3,3,3,3)),r3);
	_mm_storeu_ps(&out[0][0],_mm_add_ps(_mm_add_ps(t0,t1),_mm_add_ps(t2,t3)));

	l = _mm_loadu_ps(&b[1][0]);
	t0 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(0,0,0,0)),r0);
	t1 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(1,1,1,1)),r1);
	t2 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(2,2,2,2)),r2);
	t3 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(3,3,3,3)),r3);
	_mm_storeu_ps(&out[1][0],_mm_add_ps(_mm_add_ps(t0,t1),_mm_add_ps(t2,t3)));

	l = _mm_loadu_ps(&b[2][0]);
	t0 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(0,0,0,0)),r0);
	t1 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(1,1,1,1)),r1);
	t2 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(2,2,2,2)),r2);
	t3 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(3,3,3,3)),r3);
	_mm_storeu_ps(&out[2][0],_mm_add_ps(_mm_add_ps(t0,t1),_mm_add_ps(t2,t3)));

	l = _mm_loadu_ps(&b[3][0]);
	t0 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(0,0,0,0)),r0);
	t1 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(1,1,1,1)),r1);
	t2 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(2,2,2,2)),r2);
	t3 = _mm_mul_ps(_mm_shuffle_ps(l,l,_MM_SHUFFLE(3,3,3,3)),r3);
	_mm_storeu_ps(&out[3][0], _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3)));
#else
	out = a * b;
#endif
}
