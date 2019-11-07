#pragma once

#include "Utility/helpers.h"

#include "Renderer/GLDriver.h"

class Program
{
	friend class Material;

public:
	/// Method (public)
	template <typename T>
	static void setBuiltin(std::string name, T value)
	{
		auto it = builtins_names.find(name);
		if (it != builtins_names.end())
			set(it->second, value);
		else
			std::cout << "Unknown builtin: " << name << std::endl;
	}

	template <typename T>
	static inline void set(size_t builtin, T value)
	{
		uint8_t *b = builtins.data() + builtin;
		(*b)++; // Bump update index;

		b += sizeof(uint8_t) + sizeof(GLuint);

		memcpy(b, &value, sizeof(T));
	}

	void updateBuiltins();

	/// Methods (static)
	static Program* get(std::string _name);
	static Program* getDefault();
	static void init();
	static void clear();

private:
	/// Methods (private)
	Program(std::string _name);
	~Program();

	void link();
	void load_uniforms();

	void disass() const;

	/// Attributes (static)
	static std::map<std::string, Program*> programs;

	/// Attributes
	struct Shader;
	Shader *vertex, *fragment;

	unsigned program;
	std::string name;


	struct Uniform
	{
		GLuint location, type, size;
		size_t offset; // offset in the array 'uniforms' of each Material
	};
	struct Builtin
	{
		GLuint location;
		uint8_t update_idx;
		size_t offset; // offset in the static array 'builtins'
	};

	std::vector<Uniform> uniforms;
	std::map<std::string, size_t> uniforms_names;

	std::vector<Builtin> builtins_used;

	static std::vector<uint8_t> builtins; // Contains < update_idx | type | data > sequenced for each builtin
	static std::map<std::string, size_t> builtins_names;
	static void addBuiltin(std::string name, GLuint type);


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

inline void set_uniform(unsigned location, GLuint type, const void *data)
{
	switch (type)
	{
	case GL_FLOAT:
		glCheck(glUniform1f(location, *(const float*)data));
		break;
	case GL_FLOAT_VEC2:
		glCheck(glUniform2fv(location, 1, (float*)data));
		break;
	case GL_FLOAT_VEC3:
		glCheck(glUniform3fv(location, 1, (float*)data));
		break;
	case GL_FLOAT_VEC4:
		glCheck(glUniform4fv(location, 1, (float*)data));
		break;
	case GL_FLOAT_MAT3:
		glCheck(glUniformMatrix3fv(location, 1, GL_FALSE, (float*)data));
		break;
	case GL_FLOAT_MAT4:
		glCheck(glUniformMatrix4fv(location, 1, GL_FALSE, (float*)data));
		break;
	}
}
