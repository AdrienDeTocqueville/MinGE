#pragma once

#include <SFML/System.hpp>
#include "Utility/glm.h"

const float PI = 3.14159265358979323846f;
const float EPSILON = 0.0001f;


inline vec2 toVec2(sf::Vector2i v)	{ return vec2(v.x, v.y); }
inline vec2 toVec2(sf::Vector2u v)	{ return vec2(v.x, v.y); }
inline sf::Vector2i toSFVec2i(vec2 v)	{ return sf::Vector2i((int)v.x, (int)v.y); }
inline sf::Vector2i toSFVec2i(ivec2 v)	{ return sf::Vector2i(v.x, v.y); }
inline sf::Vector2u toSFVec2u(ivec2 v)	{ return sf::Vector2u(v.x, v.y); }


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
