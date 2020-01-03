#include "Renderer/GLDriver.h"

inline void set_uniform(int location, int x)
{
	glCheck(glUniform1i(location, x));
}

inline void set_uniform(int location, GLuint type, GLsizei num, const void *data)
{
	switch (type)
	{
	case GL_FLOAT:
		glCheck(glUniform1f(location, *(const float*)data));
		break;
	case GL_FLOAT_VEC2:
		glCheck(glUniform2fv(location, num, (float*)data));
		break;
	case GL_FLOAT_VEC3:
		glCheck(glUniform3fv(location, num, (float*)data));
		break;
	case GL_FLOAT_VEC4:
		glCheck(glUniform4fv(location, num, (float*)data));
		break;
	case GL_FLOAT_MAT3:
		glCheck(glUniformMatrix3fv(location, num, GL_FALSE, (float*)data));
		break;
	case GL_FLOAT_MAT4:
		glCheck(glUniformMatrix4fv(location, num, GL_FALSE, (float*)data));
		break;
	}
}

inline size_t Shader::getBuiltinLocation(const std::string &name)
{
	auto it = builtins_names.find(name);

#ifdef DEBUG
	if (it == builtins_names.end())
	{
		std::cout << "Unknown builtin: " << name << std::endl;
		return -1;
	}
#endif // DEBUG

	return it->second;
}

template <typename T>
inline void Shader::setBuiltin(size_t location, T value)
{
	uint8_t *b = builtins.data() + location;
	(*b)++; // Bump update index;

	b += sizeof(uint8_t) + sizeof(GLuint);

	memcpy(b, &value, sizeof(T));
}

template<>
inline void Shader::setBuiltin(size_t location, const class Texture *value)
{
	uint8_t *b = builtins.data() + location;
	(*b)++; // Bump update index;

	b += sizeof(uint8_t) + sizeof(GLuint);

	*(Texture**)(b) = (Texture*)value;
}

template <typename T>
inline void Shader::setBuiltin(std::string name, T value)
{
	setBuiltin(getBuiltinLocation(name), value);
}
