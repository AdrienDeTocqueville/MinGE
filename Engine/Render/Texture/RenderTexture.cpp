#include "Render/Texture/RenderTexture.h"
#include "Render/GLDriver.h"

void render_texture_t::create(ivec2 size, Format format)
{
	if (handle == 0) glGenRenderbuffers(1, &handle);

	GLenum formats[] = { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH24_STENCIL8, GL_DEPTH_COMPONENT32 };
	GLenum gl_format = formats[format];

	glBindRenderbuffer(GL_RENDERBUFFER, handle);
	glCheck(glRenderbufferStorage(GL_RENDERBUFFER, gl_format, size.x, size.y));
}

void render_texture_t::destroy()
{
	glDeleteRenderbuffers(1, &handle);
	handle = 0;
}
