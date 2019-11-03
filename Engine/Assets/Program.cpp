#include "Assets/Program.h"
#include "Utility/Error.h"

#include <fstream>

std::map<std::string, Program*> Program::programs;


Program::Program(std::string _name):
	program(0),
	name(_name),
	vertex(nullptr), fragment(nullptr)
{
	if (!(vertex = Shader::get(GL_VERTEX_SHADER, _name + ".vert")))
		return;

	if (!(fragment = Shader::get(GL_FRAGMENT_SHADER, _name + ".frag")))
		return;

	link();
}

/// Destructor (private)
Program::~Program()
{
	glCheck(glDeleteProgram(program));
}

/// Methods (private)
void Program::link()
{
	program = glCreateProgram();

	/// Link

	if (vertex)
		glCheck(glAttachShader(program, vertex->shader));
	if (fragment)
		glCheck(glAttachShader(program, fragment->shader));

	glCheck(glLinkProgram(program));

	if (vertex)
		glCheck(glDetachShader(program, vertex->shader));
	if (fragment)
		glCheck(glDetachShader(program, fragment->shader));

	///Detect errors

	GLint succes(0);
	glCheck(glGetProgramiv(program, GL_LINK_STATUS, &succes));

	if (succes != GL_TRUE)
	{
		GLint stringSize(0);
		glCheck(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &stringSize));

		char* error = new char[stringSize + 1];

		glCheck(glGetShaderInfoLog(program, stringSize, &stringSize, error));
		error[stringSize] = '\0';

		std::string errorString(reinterpret_cast<char*>(error));
		Error::add(OPENGL_ERROR, "Shader::linkProgram() -> " + errorString);

		delete[] error;
		glCheck(glDeleteProgram(program));
		program = 0;

		return;
	}

	// Bind uniforms

	int ub_count = 0, custom_ub_count = 0;
	glCheck(glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &ub_count));
	blocks.reserve(ub_count);

	for (int i = 0; i < ub_count; ++i)
	{
		const int MAX_NAME_LENGTH = 255;
		char c_name[MAX_NAME_LENGTH];
		int name_len;
		glCheck(glGetActiveUniformBlockName(program, (GLuint)i, MAX_NAME_LENGTH, &name_len, c_name));
		std::string name(c_name, name_len);

		// Find binding slot
		static const std::map<std::string, unsigned> bindings = {
			{"Camera", 0},
			{"Light", 1},
		};

		unsigned binding;
		auto it = bindings.find(name);
		if (it != bindings.end())
			binding = it->second;
		else
			binding = bindings.size() + (custom_ub_count++);

		// Bind uniform block
		unsigned index = glGetUniformBlockIndex(program, c_name);
		glUniformBlockBinding(program, index, binding);

		// Keep track of the block
		int size;
		glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &size);

		blocks.emplace_back(std::move(name), binding, index, size);

		// Query block content
		int uniform_count;
		glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniform_count);

		std::vector<GLuint> indices(uniform_count);
		glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (GLint*)indices.data());

		std::vector<GLint> types(uniform_count);
		glGetActiveUniformsiv(program, uniform_count, indices.data(), GL_UNIFORM_TYPE, types.data());

		std::vector<GLint> offsets(uniform_count);
		glGetActiveUniformsiv(program, uniform_count, indices.data(), GL_UNIFORM_OFFSET, offsets.data());

		auto& uniforms = blocks.back().uniforms;
		for (int i = 0; i < uniform_count; ++i)
		{
			glGetActiveUniformName(program, indices[i], MAX_NAME_LENGTH, &name_len, c_name);
			name = std::string(c_name, name_len);

			uniforms.emplace_back(std::move(name), indices[i], types[i], offsets[i]);
		}
	}
}

/// Methods (static)
Program* Program::get(std::string _name)
{
	auto it = programs.find(_name);
	if (it != programs.end())
		return it->second;

	Program *prgm = new Program(_name);
	if (!prgm->program)
	{
		delete prgm;
		return nullptr;
	}

	programs.emplace(_name, prgm);
	return prgm;
}

Program* Program::getDefault()
{
	return Program::get("object");
}

void Program::clear()
{
	Shader::clear();

	for (auto& entry: programs)
		delete entry.second;

	programs.clear();
}

/// Methods (public)
void Program::use()
{
	GL::UseProgram(program);
}

/*
void Program::send(unsigned _location, int _value) const
{
	glCheck(glUniform1i(locations[_location], _value));
}
void Program::send(unsigned _location, unsigned _value) const
{
	glCheck(glUniform1i(locations[_location], (int)_value));
}
void Program::send(unsigned _location, float _value) const
{
	glCheck(glUniform1f(locations[_location], _value));
}
void Program::send(unsigned _location, vec3 _value) const
{
	glCheck(glUniform3f(locations[_location], _value[0], _value[1], _value[2]));
}
void Program::send(unsigned _location, vec4 _value) const
{
	glCheck(glUniform4f(locations[_location], _value[0], _value[1], _value[2], _value[3]));
}
void Program::send(unsigned _location, mat3 _value) const
{
	glCheck(glUniformMatrix3fv(locations[_location], 1, GL_FALSE, value_ptr(_value)));
}
void Program::send(unsigned _location, mat4 _value) const
{
	glCheck(glUniformMatrix4fv(locations[_location], 1, GL_FALSE, value_ptr(_value)));
}
void Program::send(unsigned _location, const std::vector<mat4>& _values) const
{
	glCheck(glUniformMatrix4fv(locations[_location], _values.size(), GL_FALSE, value_ptr(_values[0])));
}
*/

const std::vector<UniformBlock>& Program::getBlocks() const
{
	return blocks;
}

/// Shader Class
std::unordered_map<std::string, Program::Shader*> Program::Shader::shaders;

Program::Shader* Program::Shader::load(GLuint type, std::string& _shader)
{
	std::ifstream file(("Shaders/" + _shader).c_str());

	if(!file)
	{
		Error::add(FILE_NOT_FOUND, "Shader::load() -> Shaders/" + _shader);
		return nullptr;
	}


	GLuint shaderID = glCreateShader(type);

	if(!shaderID)
	{
		Error::add(OPENGL_ERROR, "Shader::load() -> glCreateShader() returns 0");
		return nullptr;
	}

	Shader* shader = new Shader(shaderID);

	std::string line, sourceCode;
	while(getline(file, line))
		sourceCode += line + '\n';

	const GLchar* charSource = sourceCode.c_str();

	glCheck(glShaderSource(shaderID, 1, &charSource, nullptr));
	glCheck(glCompileShader(shaderID));

	GLint success;
	glCheck(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success));

	if (success != GL_TRUE)
	{
		GLint stringSize(0);
		glCheck(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &stringSize));

		char *error = new char[stringSize + 1];

		glCheck(glGetShaderInfoLog(shaderID, stringSize, &stringSize, error));
		error[stringSize] = '\0';

		Error::add(OPENGL_ERROR, "Shader::compileShader() -> Compilation error in file: " + _shader);
		std::cout << std::endl << "\"" + _shader + "\" error = " << error;

		delete[] error;
		delete shader;

		return nullptr;
	}

	shaders[_shader] = shader;
	return shader;
}

Program::Shader* Program::Shader::get(GLuint type, std::string shader)
{
	auto it = shaders.find(shader);
	if (it == shaders.end())
		return load(type, shader);
	else
		return it->second;
}

void Program::Shader::clear()
{
	for (auto it = shaders.begin() ; it != shaders.end() ; ++it)
		delete it->second;

	shaders.clear();
}
