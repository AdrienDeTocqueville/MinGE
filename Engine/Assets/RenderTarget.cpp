#include "Assets/RenderTarget.h"
#include "Systems/GraphicEngine.h"

#include "Utility/IO/Input.h"
#include "Utility/Error.h"

std::weak_ptr<RenderTarget> RenderTarget::basic;

RenderTargetRef RenderTarget::create(uvec2 size, Depth depth, unsigned priority)
{
	unsigned fbo;
	glGenFramebuffersEXT(1, &fbo);

	/// Color attachment
	Texture colorBuffer;
	colorBuffer.create(size);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer.getId(), 0);

	/// Depth attachement
	RenderBuffer depthBuffer;
	if (depth)
	{
		depthBuffer.create(size, depth);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer.getId());
	}

	int val = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if (val != GL_FRAMEBUFFER_COMPLETE)
	{
		Error::add(OPENGL_ERROR, "RenderTarget::create() -> glCheckFramebufferStatus() returns: " + val);
		glDeleteFramebuffers(1, &fbo);
		return nullptr;
	}

	GLuint buf[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buf);

	return RenderTargetRef(new RenderTarget(size, fbo, std::move(colorBuffer), std::move(depthBuffer), 0));
}

RenderTargetRef RenderTarget::getDefault()
{
	if (auto shared = basic.lock())
		return shared;

	RenderTargetRef shared(new RenderTarget(Input::getWindowSize()));

	basic = shared;
	return shared;
}

RenderTarget::RenderTarget(uvec2 _size):
	size(_size), fbo(0), priority(0)
{
	bucket.target = this;
}

RenderTarget::RenderTarget(uvec2 _size, unsigned _fbo, Texture &&_color, RenderBuffer &&_depth, unsigned _priority):
	size(_size), fbo(_fbo), colorBuffer(_color), depthBuffer(_depth), priority(_priority)
{
	bucket.target = this;
}


RenderTarget::~RenderTarget()
{
	glDeleteFramebuffers(1, &fbo);
}

void RenderTarget::bind() const
{
	GL::BindFramebuffer(fbo);
}

void RenderTarget::resize(uvec2 _size)
{
	// TODO: resize attachments
	GraphicEngine::get()->onResize(this);
}

const Texture* RenderTarget::getColorBuffer() const
{
	if (colorBuffer.getId())
		return &colorBuffer;
	return nullptr;
}

const RenderBuffer* RenderTarget::getDepthBuffer() const
{
	if (depthBuffer.getId())
		return &depthBuffer;
	return nullptr;
}

vec2 RenderTarget::getSize() const
{
	return size;
}

unsigned RenderTarget::getPriority() const
{
	return priority;
}
