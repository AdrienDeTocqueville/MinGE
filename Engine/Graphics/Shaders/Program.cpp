#include "Graphics/Shaders/Program.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/GLDriver.h"

#define STB_INCLUDE_IMPLEMENTATION
#define STB_INCLUDE_LINE_GLSL
#include "Graphics/Shaders/stb_include.h"

#include "Utility/Error.h"
#include "Utility/stb_sprintf.h"

#include <fstream>

struct Stages
{
	unsigned vertex;
	unsigned tess_ctrl;
	unsigned tess_eval;
	unsigned geometry;
	unsigned fragment;
};
const int NUM_STAGES = sizeof(Stages) / sizeof(unsigned);


static bool check_compile(unsigned shader, const std::string &file)
{
	GLint success;
	glCheck(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));

	if (success != GL_TRUE)
	{
#ifdef DEBUG
		GLint stringSize(0);
		glCheck(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &stringSize));

		char *error = new char[stringSize + 1];

		glCheck(glGetShaderInfoLog(shader, stringSize, &stringSize, error));
		error[stringSize] = '\0';

		std::cout << std::endl << error;
		delete[] error;
#endif
		Error::add(Error::OPENGL, "Failed to compile shader: " + file);
	}
	return success == GL_TRUE;
}

static unsigned compile(unsigned type, const std::string &path, const char *pass_define, const char *defines, const char *builtins)
{
	std::ifstream file("Assets/Shaders/" + path);
	if (!file)
	{
		Error::add(Error::FILE_NOT_FOUND, "Shader file not found: " + path);
		return 0;
	}

	char error[256];
	std::string glsl(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	char *final_source = stb_include_string(glsl.c_str(), glsl.size(), "Assets/Shaders", path.c_str(), error);
	if (final_source == NULL)
	{
		Error::add(Error::FILE_NOT_FOUND, error);
		return 0;
	}

	GLuint shader = glCreateShader(type);
	if (!shader)
	{
		free(final_source);
		Error::add(Error::OPENGL, "glCreateShader() returns 0");
		return 0;
	}

	const GLchar* source[] = {
		defines,
		pass_define,
		builtins,
		final_source,
	};

	glCheck(glShaderSource(shader, 4, source, nullptr));
	glCheck(glCompileShader(shader));

	free(final_source);

	if (!check_compile(shader, path))
	{
		glCheck(glDeleteShader(shader));
		return 0;
	}

	return shader;
}

static bool check_link(unsigned program)
{
	GLint success(0);
	glCheck(glGetProgramiv(program, GL_LINK_STATUS, &success));

	if (success != GL_TRUE)
	{
		GLint stringSize(0);
		glCheck(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &stringSize));

		char* error = new char[stringSize + 1];

		glCheck(glGetShaderInfoLog(program, stringSize, &stringSize, error));
		error[stringSize] = '\0';

		std::string errorString(reinterpret_cast<char*>(error));
		Error::add(Error::OPENGL, "Linker error: " + errorString);

		delete[] error;
	}
	return success == GL_TRUE;
}

static unsigned link(const Stages &stages)
{
	unsigned program = glCreateProgram();
	auto shaders = (unsigned*)&stages;

	for (int i(0); i < NUM_STAGES; i++)
	{
		if (shaders[i])
			glCheck(glAttachShader(program, shaders[i]));
	}

	glCheck(glLinkProgram(program));

	for (int i(0); i < NUM_STAGES; i++)
	{
		if (shaders[i])
			glCheck(glDetachShader(program, shaders[i]));
	}


	if (!check_link(program))
	{
		GL::DeleteProgram(program);
		return 0;
	}

	return program;
}

Program::Program(const ShaderSources &sources, RenderPass::Type pass, const char *defines, const char *builtins):
	program(0)
{
	#define COMPILE(type, name) (0 == (stages.name ? stages.name :\
		(stages.name = compile(type, sources.name, pass_define, defines, builtins)))) \

	static const char *pass_defines[] = {
		"SHADOW_PASS",
		"DEPTH_PASS",
		"FORWARD_PASS",
	};
	static_assert(sizeof(pass_defines) / sizeof(*pass_defines) == RenderPass::Count, "Invalid pass count");

	char pass_define[64];
	stbsp_snprintf(pass_define, 64, "#define %s\n", pass_defines[pass]);

	Stages stages;
	memset(&stages, 0, sizeof(Stages));

	bool error;
	do {
		error = false;

		if (!sources.vertex.empty())
			error |= COMPILE(GL_VERTEX_SHADER, vertex);
		if (!sources.tess_ctrl.empty())
			error |= COMPILE(GL_TESS_CONTROL_SHADER, tess_ctrl);
		if (!sources.tess_eval.empty())
			error |= COMPILE(GL_TESS_EVALUATION_SHADER, tess_eval);
		if (!sources.geometry.empty())
			error |= COMPILE(GL_GEOMETRY_SHADER, geometry);
		if (!sources.fragment.empty())
			error |= COMPILE(GL_FRAGMENT_SHADER, fragment);

		if (error)
		{
			auto answer = Error::ask(Error::USER, "Failed to compile shader. Retry ?");
			if (answer == Error::CANCEL)	exit(EXIT_FAILURE);
			if (answer == Error::NO)	error = false;
		}
	}
	while (error);

	program = link(stages);

	auto shaders = (unsigned*)&stages;
	for (int i(0); i < NUM_STAGES; i++)
		glCheck(glDeleteShader(shaders[i]));
}

Program::~Program()
{
	GL::DeleteProgram(program);
}

