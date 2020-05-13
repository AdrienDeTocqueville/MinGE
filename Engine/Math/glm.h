#pragma once

//#define GLM_FORCE_AVX2
//#define GLM_FORCE_ALIGNED

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <iostream>

using namespace glm;

const float PI = 3.14159265358979323846f;
const float EPSILON = 0.0001f;

void simd_mul(mat4& out, const mat4& a, const mat4& b);

template<length_t L, typename T, qualifier Q>
std::ostream& operator<<(std::ostream& out, const vec<L, T, Q>& v)
{
	out << '(';
	for (unsigned i(0); i < L; ++i)
		out << v[i] << (i == L-1 ? ")" : ", ");
	return out;
}

template<length_t R, length_t C, typename T, qualifier Q>
std::ostream& operator<<(std::ostream& out, const mat<R, C, T, Q>& m)
{
	out << '(' << m[0];
	for (unsigned i(1); i < R; ++i)
	{
		out << "\n " << m[i];
	}
	return out << ')';
}


inline bool epsilonEqual(const float& a, const float& b)
{
	return epsilonEqual(a, b, EPSILON);
}

template<length_t L, typename T, qualifier Q>
inline bool epsilonEqual(vec<L, T, Q> const& a, vec<L, T, Q> const& b, T const& epsilon = EPSILON)
{
	for (unsigned i(0); i < L; ++i)
	if (!epsilonEqual(a[i], b[i], epsilon))
		return false;

	return true;
}

inline bool epsilonEqual(const quat& a, const quat& b, float epsilon = EPSILON)
{
	for (unsigned i(0); i < 4; i++)
	if (!epsilonEqual(a[i], b[i], epsilon))
		return false;

	return true;
}
