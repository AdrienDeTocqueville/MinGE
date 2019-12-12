#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <GL/glew.h>

#include "Utility/glm.h"

#ifdef DEBUG
#include <iostream>
// Macro and function from SFML
#define glCheck(expr) do { expr; glCheckError(__FILE__, __LINE__, #expr); } while (false)
void glCheckError(const char* file, unsigned int line, const char* expression);
#else
#define glCheck(expr) expr
#endif

class GL
{
	struct GLState
	{
		GLuint ubo, vbo, ebo, vao, fbo;
		GLuint program;
		GLuint texture_unit;

		ivec4 view, scissor;
		vec4 clearColor;
	};

	static GLState state;

public:
	// Creation
	static GLuint GenBuffer()
	{
		GLuint buf;
		glCheck(glGenBuffers(1, &buf));
		return buf;
	}

	static GLuint GenVertexArray()
	{
		GLuint vao;
		glCheck(glGenVertexArrays(1, &vao));
		return vao;
	}

	// Deletion
	static void DeleteBuffer(GLuint buf)
	{
		glCheck(glDeleteBuffers(1, &buf));
		if (state.ubo == buf)
			state.ubo = 0;
		else if (state.vbo == buf)
			state.vbo = 0;
		else if (state.ebo == buf)
			state.ebo = 0;
	}

	static void DeleteVertexArray(GLuint vao)
	{
		glCheck(glDeleteVertexArrays(1, &vao));
		if (state.vao == vao)
			state.vao = 0;
	}

	// Binding
	static void BindUniformBuffer(GLuint buf)
	{
		if (buf != state.ubo)
		{
			glCheck(glBindBuffer(GL_UNIFORM_BUFFER, buf));
			state.ubo = buf;
		}
	}

	static void BindBufferRange(GLuint binding, GLuint buf, GLintptr offset, GLsizeiptr size)
	{
		glCheck(glBindBufferRange(GL_UNIFORM_BUFFER, binding, buf, offset, size));
		state.ubo = buf;
	}

	static void BindVertexBuffer(GLuint buf)
	{
		if (buf != state.vbo)
		{
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, buf));
			state.vbo = buf;
		}
	}

	static void BindElementBuffer(GLuint buf)
	{
		if (buf != state.ebo)
		{
			glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf));
			state.ebo = buf;
		}
	}

	static void BindVertexArray(GLuint vao)
	{
		if (vao != state.vao)
		{
			glCheck(glBindVertexArray(vao));
			state.vao = vao;
		}
	}

	static void BindFramebuffer(GLuint buf)
	{
		if (buf != state.fbo)
		{
			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, buf));
			state.fbo = buf;
		}
	}

	static void UseProgram(GLuint prog)
	{
		if (prog != state.program)
		{
			glCheck(glUseProgram(prog));
			state.program = prog;
		}
	}

	// Change state
	static void ActiveTexture(GLuint slot)
	{
		if (slot != state.texture_unit)
		{
			glActiveTexture(slot);
			state.texture_unit = slot;
		}
	}

	static void Viewport(const ivec4 &view)
	{
		if (view != state.view)
		{
			glViewport(view.x, view.y, view.z, view.w);
			state.view = view;
		}
	}

	static void Scissor(const ivec4 &scissor)
	{
		if (scissor != state.scissor)
		{
			glScissor(scissor.x, scissor.y, scissor.z, scissor.w);
			state.scissor = scissor;
		}
	}


	static void ClearColor(const vec4 &color)
	{
		if (color != state.clearColor)
		{
			glClearColor(color.r, color.g, color.b, color.a);
			state.clearColor = color;
		}
	}
};
