#include "Assets/RenderTarget.h"
#include "Systems/GraphicEngine.h"

#include "Utility/IO/Input.h"
#include "Utility/Error.h"

std::weak_ptr<RenderTarget> RenderTarget::basic;

RenderTargetRef RenderTarget::create(uvec2 size, DepthType depth)
{
	unsigned fbo = GL::GenFramebuffer();
	GL::BindFramebuffer(fbo);

	/// Color attachment
	Texture colorBuffer;
	colorBuffer.create(size);
	glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer.getId(), 0));

	GLuint buf[1] = {GL_COLOR_ATTACHMENT0};
	glCheck(glDrawBuffers(1, buf));

	/// Depth attachement
	RenderBuffer depthBuffer;
	if (depth)
	{
		depthBuffer.create(size, depth);
		glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer.getId()));
	}

	// Check
	int val = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (val != GL_FRAMEBUFFER_COMPLETE)
	{
		Error::add(Error::OPENGL, "RenderTarget::create() -> glCheckFramebufferStatus() returns: " + val);
		GL::DeleteFramebuffer(fbo);
		return nullptr;
	}

	return RenderTargetRef(new RenderTarget(size, fbo, std::move(colorBuffer), std::move(depthBuffer)));
}

RenderTargetRef RenderTarget::create_depth_map(uvec2 size)
{
	unsigned fbo = GL::GenFramebuffer();
	GL::BindFramebuffer(fbo);

	/// Depth attachement
	Texture depth_tex;
	depth_tex.create(size, GL_DEPTH_COMPONENT, GL_FLOAT);
	glCheck(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex.getId(), 0));

	/// Color attachment
	glCheck(glDrawBuffer(GL_NONE));
	glCheck(glReadBuffer(GL_NONE));

	// Check
	int val = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (val != GL_FRAMEBUFFER_COMPLETE)
	{
		Error::add(Error::OPENGL, "RenderTarget::create_depth_map() -> glCheckFramebufferStatus() returns: " + val);
		GL::DeleteFramebuffer(fbo);
		return nullptr;
	}

	return RenderTargetRef(new RenderTarget(size, fbo, std::move(depth_tex), std::move(RenderBuffer())));
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
	fbo(0), size(_size)
{ }

RenderTarget::RenderTarget(uvec2 _size, unsigned _fbo, Texture &&_color, RenderBuffer &&_depth):
	fbo(_fbo), size(_size), colorBuffer(std::move(_color)), depthBuffer(std::move(_depth))
{ }


RenderTarget::~RenderTarget()
{
	GL::DeleteFramebuffer(fbo);
}

void RenderTarget::resize(uvec2 _size)
{
	// TODO: resize attachments
	(void)_size;
	Error::add(Error::MINGE, "render target will not be resized");
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
