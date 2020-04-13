#pragma once

#include "Renderer/Renderer.h"
#include "Utility/Error.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <GL/glew.h>

namespace Renderer
{

void init()
{
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

	//Debug::init();
}

}
