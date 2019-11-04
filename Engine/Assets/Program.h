#pragma once

#include "Utility/helpers.h"

#include "Renderer/GLDriver.h"

struct Uniform
{
	GLuint location, size, type;
	GLint num;
};

class Program
{
	friend class Material;

public:
	/// Methods (static)
	static Program* get(std::string _name);
	static Program* getDefault();
	static void clear();

private:
	/// Methods (private)
	Program(std::string _name);
	~Program();

	void link();
	void load_uniforms();

	/// Attributes (static)
	static std::map<std::string, Program*> programs;

	/// Attributes
	struct Shader;
	Shader *vertex, *fragment;

	unsigned program;
	std::string name;

	std::map<std::string, Uniform> uniforms;


	struct Shader
	{
		static Shader* get(GLuint type, std::string shader);
		static Shader* load(GLuint type, std::string& _shader);
		static void clear();

		Shader(unsigned _shader): shader(_shader)
		{ }

		~Shader()
		{ glDeleteShader(shader); }

		unsigned shader;

		static std::unordered_map<std::string, Shader*> shaders;
	};
};

inline void set_uniform(unsigned _location, int _value)
{
	glCheck(glUniform1i(_location, _value));
}
inline void set_uniform(unsigned _location, unsigned _value)
{
	glCheck(glUniform1i(_location, (int)_value));
}
inline void set_uniform(unsigned _location, float _value)
{
	glCheck(glUniform1f(_location, _value));
}
inline void set_uniform(unsigned _location, vec3 _value)
{
	glCheck(glUniform3f(_location, _value[0], _value[1], _value[2]));
}
inline void set_uniform(unsigned _location, vec4 _value)
{
	glCheck(glUniform4f(_location, _value[0], _value[1], _value[2], _value[3]));
}
inline void set_uniform(unsigned _location, mat3 _value)
{
	glCheck(glUniformMatrix3fv(_location, 1, GL_FALSE, value_ptr(_value)));
}
inline void set_uniform(unsigned _location, mat4 _value)
{
	glCheck(glUniformMatrix4fv(_location, 1, GL_FALSE, value_ptr(_value)));
}
inline void set_uniform(unsigned _location, const std::vector<mat4>& _values)
{
	glCheck(glUniformMatrix4fv(_location, _values.size(), GL_FALSE, value_ptr(_values[0])));
}
