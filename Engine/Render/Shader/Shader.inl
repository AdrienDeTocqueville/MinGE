#pragma once

#include "Render/Shader/Shader.h"
#include "Render/GLDriver.h"

inline void set_uniform(int location, int x)
{
	glCheck(glUniform1i(location, x));
}

inline void set_uniform(int location, GLuint type, GLsizei num, const void *data)
{
	switch (type)
	{
	case GL_FLOAT_VEC4:
		glCheck(glUniform4fv(location, num, (float*)data));
		break;
	case GL_FLOAT_VEC3:
		glCheck(glUniform3fv(location, num, (float*)data));
		break;
	case GL_FLOAT:
		glCheck(glUniform1f(location, *(const float*)data));
		break;
	case GL_FLOAT_MAT4:
		glCheck(glUniformMatrix4fv(location, num, GL_FALSE, (float*)data));
		break;

	case GL_FLOAT_VEC2:
		glCheck(glUniform2fv(location, num, (float*)data));
		break;
	case GL_FLOAT_MAT3:
		glCheck(glUniformMatrix3fv(location, num, GL_FALSE, (float*)data));
		break;
	case GL_INT:
		glCheck(glUniform1i(location, *(const int*)data));
		break;
	}
}

inline uint32_t Shader::get_builtin_location(const std::string &name)
{
	auto it = bindings_cache.variables.find(name);

#ifdef DEBUG
	if (it == bindings_cache.variables.end())
	{
		std::cout << "Unknown builtin: " << name << std::endl;
		return -1;
	}
#endif // DEBUG

	return it->second;
}

void Shader::set_uniform(uint32_t slot, uint32_t buf, uint32_t offset, uint32_t size)
{
	if (bindings_cache.update(BindingsCache::Uniform, slot, buf, offset, size))
		GL::BindUniformRange(slot, buf, offset, size);
}

void Shader::set_storage(uint32_t slot, uint32_t buf, uint32_t offset, uint32_t size)
{
	if (bindings_cache.update(BindingsCache::Storage, slot, buf, offset, size))
		GL::BindStorageRange(slot, buf, offset, size);
}
