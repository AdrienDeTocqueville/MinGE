#include "Math/glm.h"

#ifndef NO_SIMD
#include <immintrin.h>
#endif // NO_SIMD

void simd_mul(mat4& out, const mat4& a, const mat4& b)
{
#ifndef NO_SIMD
	const __m128 r0 = _mm_loadu_ps(&a[0][0]);
	const __m128 r1 = _mm_loadu_ps(&a[1][0]);
	const __m128 r2 = _mm_loadu_ps(&a[2][0]);
	const __m128 r3 = _mm_loadu_ps(&a[3][0]);

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
