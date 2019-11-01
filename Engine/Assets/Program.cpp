#include "Assets/Program.h"
#include "Utility/Error.h"

#include <fstream>

std::vector<Program*> Program::programs;
unsigned Program::current = 0;

Program::Program(std::string& _vertex, std::string& _fragment):
	program(0),
	vertex(nullptr), fragment(nullptr)
{
	if (!_vertex.empty())
	{
		vertex = Shader::get(_vertex);
		if (vertex == nullptr)	return;
	}

	if (!_fragment.empty())
	{
		fragment = Shader::get(_fragment);
		if (fragment == nullptr)	return;
	}

	linkProgram();

	programs.push_back(this);
}

Program::Program(Shader* _vertex, Shader* _fragment):
	vertex(_vertex), fragment(_fragment)
{
	linkProgram();

	programs.push_back(this);
}

/// Destructor (private)
Program::~Program()
{
	glCheck(glDeleteProgram(program));
}

/// Methods (private)
void Program::linkProgram()
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

	if(succes != GL_TRUE)
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

		return;
	}

	if (vertex)
		for (unsigned i(0) ; i < vertex->uniforms.size() ; ++i)
			addLocation(getLocation(vertex->uniforms[i]));

	if (fragment)
		for (unsigned i(0) ; i < fragment->uniforms.size() ; ++i)
			addLocation(getLocation(fragment->uniforms[i]));
}

/// Methods (static)
Program* Program::get(std::string _vertex, std::string _fragment)
{
	Shader *vertex = nullptr, *fragment = nullptr;

	if (!_vertex.empty())
		vertex = Shader::get(_vertex);

	if (!_fragment.empty())
		fragment = Shader::get(_fragment);


	for (unsigned i(0) ; i < programs.size() ; i++)
		if (programs[i]->vertex->shader == vertex->shader && programs[i]->fragment->shader == fragment->shader)
			return programs[i];

	return new Program(vertex, fragment);
}

void Program::clear()
{
	Shader::clear();

	for (unsigned i(0) ; i < programs.size() ; ++i)
		delete programs[i];

	programs.clear();
}

/// Methods (public)
void Program::use()
{
	if (program == current)
		return;

	current = program;

	glCheck(glUseProgram(program));
}

GLuint Program::getLocation(const std::string& _name) const
{
	GLint loc = glGetUniformLocation(program, _name.c_str());

	if (loc == -1)
		Error::add(OPENGL_ERROR, "Program::getLocation() -> Invalid uniform name: " + _name);

	return loc;
}

void Program::addLocation(GLuint _location)
{
	locations.push_back(_location);
}

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

/// Shader Class
std::unordered_map<std::string, Program::Shader*>			 Program::Shader::shaders;
std::unordered_map<std::string, Program::Shader*>::iterator   Program::Shader::it;

Program::Shader* Program::Shader::load(std::string& _shader)
{
	std::string extension = _shader.substr(_shader.size()-5);
	GLuint shaderType;

	if (extension == ".vert")
		shaderType = GL_VERTEX_SHADER;
	else if (extension == ".frag")
		shaderType = GL_FRAGMENT_SHADER;
	else
		return nullptr;

	std::ifstream file(("Shaders/" + _shader).c_str());

	if(!file)
	{
		Error::add(FILE_NOT_FOUND, "Shader::load() -> Shaders/" + _shader);
		return nullptr;
	}


	GLuint shaderID = glCreateShader(shaderType);

	if(!shaderID)
	{
		Error::add(OPENGL_ERROR, "Shader::load() -> glCreateShader() returns 0");
		return nullptr;
	}

	Shader* shader = new Shader(shaderID);


	std::istringstream stream;

	std::string line, sourceCode;
	std::string type, varName;

	while(getline(file, line))
	{
		sourceCode += line + '\n';

		if (!line.size())  continue;

		stream.clear();
		stream.str(line);
		stream >> type;

		if (type == "uniform")
		{
			stream >> type >> varName;
			shader->uniforms.push_back(shader->getVarName(varName));
		}
	}

	const GLchar* charSource = sourceCode.c_str();

	glCheck(glShaderSource(shaderID, 1, &charSource, nullptr));

	glCheck(glCompileShader(shaderID));

	GLint success(0);
	glCheck(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success));

	if(success != GL_TRUE)
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

Program::Shader* Program::Shader::get(std::string& _shader)
{
	if ((it = shaders.find(_shader)) == shaders.end())
		return load(_shader);
	else
		return it->second;
}

void Program::Shader::clear()
{
	for (it = shaders.begin() ; it != shaders.end() ; ++it)
		delete it->second;

	shaders.clear();
}



std::string Program::Shader::getVarName(std::string str)
{
	std::size_t found = str.find("[");
	if (found != std::string::npos)
		return str.substr(0, found);

	return str.substr(0, str.size()-1);

}
