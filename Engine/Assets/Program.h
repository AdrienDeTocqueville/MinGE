#pragma once

#include "Utility/helpers.h"

#include "Renderer/GLDriver.h"

struct Uniform
{
	std::string name;
	GLuint index;
	GLint type;
	GLint offset;

	Uniform(std::string&& _name, GLuint i, GLint t, GLint o):
	name(_name), index(i), type(t), offset(o) {}
};

struct UniformBlock
{
	std::string name;
	unsigned binding, index;
	std::vector<Uniform> uniforms;
	int size;

	UniformBlock(std::string&& _name, unsigned b, unsigned i, int s):
	name(_name), binding(b), index(i), size(s) {}
};

class Program
{
	friend class Material;

public:
	/// Methods (static)
	static Program* get(std::string _name);
	static Program* getDefault();
	static void clear();

	/// Methods (public)
	void use();

	/*
	void send(unsigned _location, int _value) const;
	void send(unsigned _location, unsigned _value) const;
	void send(unsigned _location, float _value) const;
	void send(unsigned _location, vec3 _value) const;
	void send(unsigned _location, vec4 _value) const;
	void send(unsigned _location, mat3 _value) const;
	void send(unsigned _location, mat4 _value) const;
	void send(unsigned _location, const std::vector<mat4>& _values) const;
	*/

	const std::vector<UniformBlock>& getBlocks() const;

private:
	/// Methods (private)
	Program(std::string _name);
	~Program();

	void link();

	/// Attributes (static)
	static std::map<std::string, Program*> programs;

	/// Attributes
	struct Shader;
	Shader *vertex, *fragment;

	unsigned program;
	std::string name;

	std::vector<UniformBlock> blocks;


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
