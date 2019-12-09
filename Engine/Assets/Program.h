#pragma once

#include "Utility/helpers.h"

#include "Renderer/GLDriver.h"

class Program
{
	friend class Material;

public:
	/// Method (public)
	void updateBuiltins();

	/// Methods (static)
	static Program* get(std::string _name);
	static Program* getDefault();
	static void init();
	static void clear();

	static size_t getLocation(const std::string &name);

	template <typename T>
	static void setBuiltin(std::string name, T value)
	{
		set(getLocation(name), value);
	}

	template <typename T>
	static inline void set(size_t location, T value)
	{
		uint8_t *b = builtins.data() + location;
		(*b)++; // Bump update index;

		b += sizeof(uint8_t) + sizeof(GLuint);

		memcpy(b, &value, sizeof(T));
	}

private:
	/// Methods (private)
	Program(std::string _name);
	~Program();

	void link();
	void load_uniforms();
	void reload();

	void disass() const;

	/// Attributes (static)
	static std::unordered_map<std::string, Program*> programs;

	/// Attributes
	struct Shader;
	Shader *vertex, *fragment;

	unsigned program;
	std::string name;


	struct Uniform
	{
		GLint location;
		GLuint type, size;
		GLsizei num;
		size_t offset; // offset in the array 'uniforms' of each Material
	};
	struct Builtin
	{
		GLint location;
		uint8_t update_idx;
		size_t offset; // offset in the static array 'builtins'
	};

	std::vector<Uniform> uniforms;
	std::unordered_map<std::string, size_t> uniforms_names;

	std::vector<Builtin> builtins_used;

	static std::vector<uint8_t> builtins; // Contains < update_idx | type | data > sequenced for each builtin
	static std::unordered_map<std::string, size_t> builtins_names;
	static void addBuiltin(std::string name, GLuint type);


	struct Shader
	{
		static Shader* get(GLuint type, std::string shader);
		static Shader* load(GLuint type, const std::string& _shader);
		static void clear();

		Shader(unsigned _shader): shader(_shader)
		{ }

		~Shader()
		{ glDeleteShader(shader); }

		unsigned shader;

		static std::unordered_map<std::string, Shader*> shaders;
	};
};

inline void set_uniform(int _location, int _value)
{
	glCheck(glUniform1i(_location, _value));
}
inline void set_uniform(int _location, unsigned _value)
{
	glCheck(glUniform1i(_location, (int)_value));
}
inline void set_uniform(int _location, float _value)
{
	glCheck(glUniform1f(_location, _value));
}
inline void set_uniform(int _location, vec3 _value)
{
	glCheck(glUniform3f(_location, _value[0], _value[1], _value[2]));
}
inline void set_uniform(int _location, vec4 _value)
{
	glCheck(glUniform4f(_location, _value[0], _value[1], _value[2], _value[3]));
}
inline void set_uniform(int _location, mat3 _value)
{
	glCheck(glUniformMatrix3fv(_location, 1, GL_FALSE, value_ptr(_value)));
}
inline void set_uniform(int _location, mat4 _value)
{
	glCheck(glUniformMatrix4fv(_location, 1, GL_FALSE, value_ptr(_value)));
}
inline void set_uniform(int _location, const std::vector<mat4>& _values)
{
	glCheck(glUniformMatrix4fv(_location, _values.size(), GL_FALSE, value_ptr(_values[0])));
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
