#pragma once

#include "Utility/helpers.h"

#include "Renderer/GLDriver.h"

struct Uniform
{
	std::string name;
	GLuint location;
	GLuint type;
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

	/// Methods (public)
	void use();

	const std::vector<Uniform>& getUniforms() const;

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

	std::vector<Uniform> uniforms;


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
