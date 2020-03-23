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


void simd_mul(mat4& out, const mat4& a, const mat4& b);
