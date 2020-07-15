#include "Render/Shader/Program.h"
#include "Render/Shader/Shader.h"
#include "Render/GLDriver.h"

#define STB_INCLUDE_IMPLEMENTATION
#define STB_INCLUDE_LINE_GLSL
#include "Render/Shader/stb_include.h"

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


Program::~Program()
{
	GL::DeleteProgram(program);
}

static bool check_compile(unsigned shader, const char *file)
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
		Error::addf(Error::OPENGL, "Failed to compile shader: %s", file);
	}
	return success == GL_TRUE;
}

static unsigned compile(unsigned type, const std::string &path, const char *pass_define, const char *defines, const char *builtins)
{
	std::ifstream file("Assets/Shaders/" + path);
	if (!file)
	{
		Error::addf(Error::FILE_NOT_FOUND, "Shader file not found: %s", path.c_str());
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

	if (!check_compile(shader, path.c_str()))
	{
		glCheck(glDeleteShader(shader));
		return 0;
	}

	size_t start = path.rfind('/');
	if (start == std::string::npos) start = 0;
	else start++;
	size_t end = path.find('.', start);
	size_t length = end == std::string::npos ? -1 : end - start;
	glCheck(glObjectLabel(GL_SHADER, shader, length, path.c_str() + start));

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

		Error::addf(Error::OPENGL, "Linker error: %s", error);

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

static inline int compile_stages(const ShaderSources &sources, RenderPass::Type pass, const char *defines, const char *builtins, Stages &stages)
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
			if (answer != Error::Retry)	return answer;
		}
	}
	while (error);

	return Error::None;
}

Program::Program(const struct ShaderSources &sources, RenderPass::Type pass, const char *defines, const char *builtins):
	program(0)
{
	Stages stages;

	auto answer = compile_stages(sources, pass, defines, builtins, stages);
	if (answer == Error::Cancel)	exit(EXIT_FAILURE);
	else if (answer == Error::None)
		program = link(stages);

	auto shaders = (unsigned*)&stages;
	for (int i(0); i < NUM_STAGES; i++)
		glCheck(glDeleteShader(shaders[i]));
}

void Program::label(const char *name, size_t len)
{
	glCheck(glObjectLabel(GL_PROGRAM, program, len, name));
}
