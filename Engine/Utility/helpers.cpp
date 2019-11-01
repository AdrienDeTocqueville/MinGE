#include "Utility/helpers.h"

#ifdef DEBUG
// https://github.com/SFML/SFML/blob/master/src/SFML/Graphics/GLCheck.cpp
void glCheckError(const char* file, unsigned int line, const char* expression)
{
    // Get the last error
    GLenum errorCode = glGetError();

    if (errorCode != GL_NO_ERROR)
    {
        std::string fileString = file;
        std::string error = "Unknown error";
        std::string description  = "No description";

        // Decode the error code
        switch (errorCode)
        {
            case GL_INVALID_ENUM:
            {
                error = "GL_INVALID_ENUM";
                description = "An unacceptable value has been specified for an enumerated argument.";
                break;
            }

            case GL_INVALID_VALUE:
            {
                error = "GL_INVALID_VALUE";
                description = "A numeric argument is out of range.";
                break;
            }

            case GL_INVALID_OPERATION:
            {
                error = "GL_INVALID_OPERATION";
                description = "The specified operation is not allowed in the current state.";
                break;
            }

            case GL_STACK_OVERFLOW:
            {
                error = "GL_STACK_OVERFLOW";
                description = "This command would cause a stack overflow.";
                break;
            }

            case GL_STACK_UNDERFLOW:
            {
                error = "GL_STACK_UNDERFLOW";
                description = "This command would cause a stack underflow.";
                break;
            }

            case GL_OUT_OF_MEMORY:
            {
                error = "GL_OUT_OF_MEMORY";
                description = "There is not enough memory left to execute the command.";
                break;
            }

            case GL_INVALID_FRAMEBUFFER_OPERATION:
            {
                error = "GL_INVALID_FRAMEBUFFER_OPERATION";
                description = "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".";
                break;
            }
        }

        // Log the error
	std::cout << "An internal OpenGL call failed in "
        	  << fileString.substr(fileString.find_last_of("\\/") + 1) << "(" << line << ")."
        	  << "\nExpression:\n   " << expression
        	  << "\nError description:\n   " << error << "\n   " << description << "\n"
        	  << std::endl;
    }
}
#endif

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

void write(vec3 _vec, bool ret)
{
	std::cout << _vec.x << ", " << _vec.y << ", " << _vec.z;
	if (ret) std::cout << std::endl;
}

bool epsilonEqual(const vec3& a, const vec3& b, float epsilon)
{
	for (unsigned i(0) ; i < 3 ; i++)
	if (!epsilonEqual(a[i], b[i], epsilon))
		return false;

	return true;
}

bool epsilonEqual(const quat& a, const quat& b, float epsilon)
{
	for (unsigned i(0) ; i < 4 ; i++)
	if (!epsilonEqual(a[i], b[i], epsilon))
		return false;

	return true;
}

#include <immintrin.h>
void simd_mul(const mat4& a, const mat4& b, mat4& out)
{
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
}
