#include "Graphics/GLDriver.h"
#include "Utility/Error.h"

GL::GLState GL::state;

void GL::init()
{
	memset(&state, 0, sizeof(GLState));
	state.clearColor = vec4(0.0f);

	if (GLenum error = glewInit() != GLEW_OK)
	{
		std::string errorString(reinterpret_cast<const char*>(glewGetErrorString(error)));
		Error::add(Error::OPENGL, "glewInit() -> Failed with error: " + errorString);
	}

#ifdef DEBUG
	printf("\n");
	printf("Opengl version: (%s)\n", glGetString(GL_VERSION));
	printf("GLSL   version: (%s)\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_SCISSOR_TEST);
	
	glDepthFunc(GL_LEQUAL);
	glPointSize(7);
	glLineWidth(3);
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
	std::cout << "An internal OpenGL call failed in "
		<< fileString.substr(fileString.find_last_of("\\/") + 1) << "(" << line << ")."
		<< "\nExpression:\n   " << expression
		<< "\nError description:\n   " << error << "\n   " << description << "\n"
		<< std::endl;
}
#endif
