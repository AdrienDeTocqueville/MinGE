#pragma once

#include <SFML/System.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <unordered_map>
#include <vector>
#include <list>
#include <map>

//#define GLM_FORCE_SSE2 // or GLM_FORCE_SSE42 if your processor supports it
//#define GLM_FORCE_ALIGNED

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_operation.hpp>


const float PI = 3.14159265358979323846f;
const float EPSILON = 0.0001f;

using namespace glm;

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

void write(vec3 _vec, bool ret = true);

bool epsilonEqual(const vec3& a, const vec3& b, float epsilon = EPSILON);
bool epsilonEqual(const quat& a, const quat& b, float epsilon = EPSILON);


void simd_mul(const mat4& a, const mat4& b, mat4& out);
