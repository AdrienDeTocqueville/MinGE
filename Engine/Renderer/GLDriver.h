#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <GL/glew.h>


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
		GLuint ubo, vbo, vao, fbo;
		GLuint program;
		GLuint texture_unit;

		/*
		 * scissor
		 * viewport
		 * every gl state ...
		 */
	};

	static GLState state;

public:
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

	static void DeleteBuffer(GLuint buf)
	{
		glCheck(glDeleteBuffers(1, &buf));
		if (state.ubo == buf)
			state.ubo = 0;
		else if (state.vbo == buf)
			state.vbo = 0;
	}
};
