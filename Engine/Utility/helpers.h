#pragma once

#include <SFML/System.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <unordered_map>
#include <vector>

#include "Utility/glm.h"

const float PI = 3.14159265358979323846f;
const float EPSILON = 0.0001f;

template <typename T>
std::string toString(T _number)
{
	std::stringstream os;
	os << _number;

	return os.str();
}

vec2 toVec2(sf::Vector2i v);
vec2 toVec2(sf::Vector2u v);
sf::Vector2i toSFVec2i(vec2 v);
sf::Vector2u toSFVec2u(vec2 v);

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

bool epsilonEqual(const quat& a, const quat& b, float epsilon = EPSILON);


void simd_mul(const mat4& a, const mat4& b, mat4& out);
