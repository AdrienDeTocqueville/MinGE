#include <SDL2/SDL.h>

#include "Render/GLDriver.h"
#include "Utility/Error.h"
#include "IO/Input.h"

SDL_GLContext GL::context = nullptr;
GL::GLState GL::state;

GLint GL::uniform_offset_alignment;
GLint GL::storage_offset_alignment;

#ifdef DEBUG
static void GLAPIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
	const char *message, const void *userParam);
#endif

void GL::init()
{
	// OpenGL 4.3 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#ifdef DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
#endif

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	context = SDL_GL_CreateContext(Input::window());

	if (GLenum error = glewInit() != GLEW_OK)
	{
		Error::addf(Error::OPENGL, "glewInit() -> Failed with error: %s", glewGetErrorString(error));
		exit(0);
	}

#ifdef DEBUG
	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PUSH_GROUP, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_POP_GROUP, GL_DONT_CARE, 0, nullptr, GL_FALSE);
	}

	printf("\n");
	printf("Opengl version: (%s)\n", glGetString(GL_VERSION));
	printf("GLSL   version: (%s)\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif

	memset(&state, 0, sizeof(GLState));
	state.clearColor = vec4(0.0f);
	state.depth_mask = true;
	state.depth_func = GL::Less;

	glCullFace(GL_BACK);
	glEnable(GL_LINE_SMOOTH);
	glPointSize(7.0f);
	glLineWidth(1.0f);

	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniform_offset_alignment);
	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &storage_offset_alignment);
}

void GL::destroy()
{
	SDL_GL_DeleteContext(context);
}


#ifdef DEBUG
#include <iostream>
// https://github.com/SFML/SFML/blob/master/src/SFML/Graphics/GLCheck.cpp
void glCheckError(const char* file, unsigned int line, const char* expression)
{
	GLenum errorCode = glGetError();
	if (errorCode == GL_NO_ERROR)
		return;

	std::string fileString = file;
	std::string error = "Unknown error";
	std::string description  = "No description";

	// Decode the error code
	switch (errorCode)
	{
	case GL_INVALID_ENUM:
		error = "GL_INVALID_ENUM";
		description = "An unacceptable value has been specified for an enumerated argument.";
		break;

	case GL_INVALID_VALUE:
		error = "GL_INVALID_VALUE";
		description = "A numeric argument is out of range.";
		break;

	case GL_INVALID_OPERATION:
		error = "GL_INVALID_OPERATION";
		description = "The specified operation is not allowed in the current state.";
		break;

	case GL_STACK_OVERFLOW:
		error = "GL_STACK_OVERFLOW";
		description = "This command would cause a stack overflow.";
		break;

	case GL_STACK_UNDERFLOW:
		error = "GL_STACK_UNDERFLOW";
		description = "This command would cause a stack underflow.";
		break;

	case GL_OUT_OF_MEMORY:
		error = "GL_OUT_OF_MEMORY";
		description = "There is not enough memory left to execute the command.";
		break;

	case GL_INVALID_FRAMEBUFFER_OPERATION:
		error = "GL_INVALID_FRAMEBUFFER_OPERATION";
		description = "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".";
		break;
	}

	// Log the error
	std::cout << "\n\n> glCheck error detected <"
		//<< fileString.substr(fileString.find_last_of("\\/") + 1) << "(" << line << ")."
		<< "\nExpression:\n   " << expression
		<< "\nError description:\n   " << error << "\n   " << description << "\n"
		<< std::endl;
}

void GLAPIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
	const char *message, const void *userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	char *source_str, *type_str;
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             source_str = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   source_str = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: source_str = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     source_str = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     source_str = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           source_str = "Other"; break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               type_str = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type_str = "Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         type_str = "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         type_str = "Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              type_str = "Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          type_str = "Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           type_str = "Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               type_str = "Other"; break;
	}

	printf("\n==== %s (%s) ===\n%s", type_str, source_str, message);
}
#endif
