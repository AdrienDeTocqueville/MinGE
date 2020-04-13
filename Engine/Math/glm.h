#pragma once

//#define GLM_FORCE_AVX2
//#define GLM_FORCE_ALIGNED

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

using namespace glm;

const float PI = 3.14159265358979323846f;
const float EPSILON = 0.0001f;

void simd_mul(mat4& out, const mat4& a, const mat4& b);

vec3 vecClamp(vec3 v);

void write(const vec3 &x, bool ret = true);
void write(const mat4 &x, bool ret = true);


inline bool epsilonEqual(const float& a, const float& b)
{
	return epsilonEqual(a, b, EPSILON);
}

template<length_t L, typename T, qualifier Q>
inline bool epsilonEqual(vec<L, T, Q> const& a, vec<L, T, Q> const& b, T const& epsilon = EPSILON)
{
	for (unsigned i(0) ; i < L ; ++i)
	if (!epsilonEqual(a[i], b[i], epsilon))
		return false;

	return true;
}

inline bool epsilonEqual(const quat& a, const quat& b, float epsilon = EPSILON)
{
	for (unsigned i(0) ; i < 4 ; i++)
	if (!epsilonEqual(a[i], b[i], epsilon))
		return false;

	return true;
}
