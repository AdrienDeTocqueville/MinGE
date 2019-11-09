#include "Assets/Program.h"
#include "Assets/Texture.h"

#include "Utility/Error.h"

#include <fstream>

std::unordered_map<std::string, size_t> Program::builtins_names;
std::vector<uint8_t> Program::builtins;

std::unordered_map<std::string, Program*> Program::programs;

static const std::unordered_map<GLuint, uint8_t> uniform_type_size = {
	{GL_FLOAT,	sizeof(float)},
	{GL_FLOAT_VEC2, sizeof(vec2)},
	{GL_FLOAT_VEC3, sizeof(vec3)},
	{GL_FLOAT_VEC4, sizeof(vec4)},
	{GL_FLOAT_MAT3, sizeof(mat3)},
	{GL_FLOAT_MAT4, sizeof(mat4)},
	{GL_SAMPLER_2D, sizeof(Texture*)},
};


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

/// Method (public)
void Program::updateBuiltins()
{
	GL::UseProgram(program);

	for (Builtin &var : builtins_used)
	{
		uint8_t *b = builtins.data() + var.offset;

		uint8_t update_idx = *b;
		if (var.update_idx == update_idx)
			continue;
		var.update_idx = update_idx;

		b += sizeof(uint8_t);
		GLuint type = *(GLuint*)b;

		b += sizeof(GLuint);

		set_uniform(var.location, type, b);
	}
}

size_t Program::getLocation(const std::string &name)
{
	auto it = builtins_names.find(name);
	if (it != builtins_names.end())
		return it->second;

	std::cout << "Unknown builtin: " << name << std::endl;
	return -1;
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

	load_uniforms();

	//disass();
}

void Program::load_uniforms()
{
	Uniform u;
	u.size = u.offset = 0;

	GLint uniform_count = 0;
	glCheck(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count));

	GLint name_len, real_len;
	glCheck(glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_len));
	char *temp_name = new char[name_len];

	for (GLint i = 0; i < uniform_count; ++i)
	{
		// TODO: handle uniform arrays
		GLint num_unused;

		glCheck(glGetActiveUniform(program, i, name_len, &real_len, &num_unused, &u.type, temp_name));
		u.location = glGetUniformLocation(program, temp_name);

		if (u.location == -1)
			continue;

		std::string name(temp_name, real_len);
		auto it = builtins_names.find(name);

		// If builtin or not
		if (it != builtins_names.end())
			builtins_used.push_back({u.location, 0, it->second});
		else
		{
			u.offset += u.size;
			u.size = uniform_type_size.at(u.type);

			uniforms_names.emplace(std::move(name), uniforms.size());
			uniforms.push_back(u);
		}
	}

	delete[] temp_name;


	/*
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
		static const std::unordered_map<std::string, unsigned> bindings = {
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
	*/
}

void Program::disass() const
{
	GLint length;
	glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);

	GLenum format;
	std::vector<char> binary(length);
	glGetProgramBinary(program, length, NULL, &format, binary.data());

	std::ofstream binaryfile(name + ".bin");
	binaryfile.write(binary.data(), length);
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


void Program::init()
{
	builtins_names.clear();
	builtins.clear();

	// Camera
	addBuiltin("MATRIX_VP", GL_FLOAT_MAT4);
	addBuiltin("cameraPosition", GL_FLOAT_VEC3);

	// Light
	addBuiltin("lightPosition", GL_FLOAT_VEC3);
	addBuiltin("diffuseColor", GL_FLOAT_VEC3);
	addBuiltin("ambientCoefficient", GL_FLOAT);
	addBuiltin("aConstant", GL_FLOAT);
	addBuiltin("aLinear", GL_FLOAT);
	addBuiltin("aQuadratic", GL_FLOAT);

	// Model
	addBuiltin("MATRIX_M", GL_FLOAT_MAT4);
	addBuiltin("MATRIX_N", GL_FLOAT_MAT4);

}

void Program::addBuiltin(std::string name, GLuint type)
{
	size_t offset = builtins.size();
	uint8_t size = uniform_type_size.at(type);

	builtins_names[name] = offset;

	/*
	 * Builtin structure
	 * uint8_t update_idx;
	 * GLuint type;
	 * uint8_t data[size];
	 */
	builtins.resize(builtins.size() + sizeof(uint8_t) + sizeof(GLuint) + size);

	uint8_t *start = builtins.data() + offset;
	*start = 0; start += sizeof(uint8_t);
	*(GLuint *)start = type;
}

void Program::clear()
{
	Shader::clear();

	for (auto& entry: programs)
		delete entry.second;

	programs.clear();
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
