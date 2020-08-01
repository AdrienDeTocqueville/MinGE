#pragma once

#include <GL/glew.h>

#include "Math/glm.h"

#ifdef DEBUG
#include <iostream>
// Macro and function from SFML
#define glCheck(expr) do { expr; glCheckError(__FILE__, __LINE__, #expr); } while (false)
void glCheckError(const char* file, unsigned int line, const char* expression);
#else
#define glCheck(expr) expr
#endif

#define NO_DRIVER_STATE_CACHE false

struct GL
{
	enum Capability { CullFace, Blend, DepthTest, ScissorTest, CAP_COUNT };
	enum DepthFunc { Never, Always, Less, LessEqual, Equal, FUNC_COUNT };

	typedef void *SDL_GLContext;
	static SDL_GLContext context;

	static GLint uniform_offset_alignment;
	static GLint storage_offset_alignment;

private:
	struct GLState
	{
		GLuint ubo, ssbo, vbo, ebo, vao, fbo;
		GLuint program;
		GLuint texture_unit;

		ivec4 view, scissor;
		vec4 clearColor;

		bool caps[GL::CAP_COUNT];
		bool depth_mask;
		DepthFunc depth_func;
	};

	static GLState state;

	static inline GLenum cap_to_gl(Capability cap)
	{
		static const GLenum gl_caps[GL::CAP_COUNT] = {
			GL_CULL_FACE, GL_BLEND, GL_DEPTH_TEST, GL_SCISSOR_TEST
		};
		return gl_caps[cap];
	}

	static inline GLenum func_to_gl(DepthFunc func)
	{
		static const GLenum gl_funcs[GL::FUNC_COUNT] = {
			GL_NEVER, GL_ALWAYS, GL_LESS, GL_LEQUAL, GL_EQUAL
		};
		return gl_funcs[func];
	}

	static void init();
	static void destroy();

	friend struct RenderEngine;

public:
	// Capabilities
	static void Enable(Capability cap)
	{
		if (state.caps[cap] == false)
		{
			glEnable(cap_to_gl(cap));
			state.caps[cap] = true;
		}
	}

	static void Disable(Capability cap)
	{
		if (state.caps[cap] == true)
		{
			glDisable(cap_to_gl(cap));
			state.caps[cap] = false;
		}
	}

	static void DepthMask(bool enable)
	{
		if (state.depth_mask != enable)
		{
			glDepthMask(enable);
			state.depth_mask = enable;
		}
	}

	static void DepthFunc(DepthFunc func)
	{
		if (state.depth_func != func)
		{
			glDepthFunc(func_to_gl(func));
			state.depth_func = func;
		}
	}

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

	static GLuint GenFramebuffer()
	{
		GLuint fbo;
		glCheck(glGenFramebuffers(1, &fbo));
		return fbo;
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

	static void DeleteFramebuffer(GLuint fbo)
	{
		glCheck(glDeleteFramebuffers(1, &fbo));
		if (state.fbo == fbo)
			GL::BindFramebuffer(0);
	}

	static void DeleteProgram(GLuint prog)
	{
		glCheck(glDeleteProgram(prog));
		if (state.program == prog)
			GL::UseProgram(0);
	}

	// Binding
	static void BindUniformBuffer(GLuint buf)
	{
		if (buf != state.ubo || NO_DRIVER_STATE_CACHE)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, buf);
			state.ubo = buf;
		}
	}

	static void BindStorageBuffer(GLuint buf)
	{
		if (buf != state.ssbo || NO_DRIVER_STATE_CACHE)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
			state.ssbo = buf;
		}
	}

	static void BindUniformRange(GLuint binding, GLuint buf, GLintptr offset, GLsizeiptr size)
	{
		glCheck(glBindBufferRange(GL_UNIFORM_BUFFER, binding, buf, offset, size));
		state.ubo = buf;
	}

	static void BindStorageRange(GLuint binding, GLuint buf, GLintptr offset, GLsizeiptr size)
	{
		glCheck(glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding, buf, offset, size));
		state.ssbo = buf;
	}

	static void BindVertexBuffer(GLuint buf)
	{
		if (buf != state.vbo || NO_DRIVER_STATE_CACHE)
		{
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, buf));
			state.vbo = buf;
		}
	}

	static void BindElementBuffer(GLuint buf)
	{
		// Disabled because it doesn't work with imgui
		// Probably because imgui recreates the vao every frame
		/*
		if (buf != state.ebo || NO_DRIVER_STATE_CACHE)
		{
			glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf));
			state.ebo = buf;
		}
		*/
		glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf));
	}

	static void BindVertexArray(GLuint vao)
	{
		if (vao != state.vao || NO_DRIVER_STATE_CACHE)
		{
			glCheck(glBindVertexArray(vao));
			state.vao = vao;
		}
	}

	static void BindFramebuffer(GLuint fbo)
	{
		if (fbo != state.fbo || NO_DRIVER_STATE_CACHE)
		{
			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
			state.fbo = fbo;
		}
	}

	static void UseProgram(GLuint prog)
	{
		if (prog != state.program || NO_DRIVER_STATE_CACHE)
		{
			glCheck(glUseProgram(prog));
			state.program = prog;
		}
	}

	// State change
	static void ActiveTexture(GLuint slot)
	{
		if (slot != state.texture_unit || NO_DRIVER_STATE_CACHE)
		{
			glCheck(glActiveTexture(slot));
			state.texture_unit = slot;
		}
	}

	static void Viewport(const ivec4 &view)
	{
		if (view != state.view || NO_DRIVER_STATE_CACHE)
		{
			glCheck(glViewport(view.x, view.y, view.z, view.w));
			state.view = view;
		}
	}

	static void Scissor(const ivec4 &scissor)
	{
		if (scissor != state.scissor || NO_DRIVER_STATE_CACHE)
		{
			glCheck(glScissor(scissor.x, scissor.y, scissor.z, scissor.w));
			state.scissor = scissor;
		}
	}


	static void ClearColor(const vec4 &color)
	{
		if (color != state.clearColor || NO_DRIVER_STATE_CACHE)
		{
			glCheck(glClearColor(color.r, color.g, color.b, color.a));
			state.clearColor = color;
		}
	}
};
