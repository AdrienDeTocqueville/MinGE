#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Transform.h"

#include "Assets/Program.h"
#include "Systems/GraphicEngine.h"
#include "Renderer/CommandBucket.h"

#include "Utility/IO/Input.h"

Camera* Camera::current = nullptr;

Camera::Camera(float _FOV, float _zNear, float _zFar, vec3 _clearColor, RenderTexture* _renderTexture, bool _orthographic, vec4 _viewport, unsigned _clearFlags):
	FOV(_FOV), zNear(_zNear), zFar(_zFar),
	order(0),
	clearColor(_clearColor), clearFlags(_clearFlags),
	orthographic(_orthographic),
	relViewport(_viewport),
	fbo(0), renderTexture(_renderTexture)
{
	setClipPlane(vec4(0, 0, 0, 1.0f));
	computeViewPort();
	createFramebuffer();

	buckets.push_back({
		viewport, fbo, mat4(1.0f),
		vec4(clearColor, 0.0f), clearFlags,
		RenderPass::Forward
	});
}

Camera::~Camera()
{
	glDeleteFramebuffers(1, &fbo);

	delete renderTexture;
}

/// Methods (public)
Camera* Camera::clone() const
{
	return new Camera(FOV, zNear, zFar, clearColor, renderTexture, orthographic, relViewport, clearFlags);
}

void Camera::use()
{
	current = this;

	glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
	glScissor (viewport.x, viewport.y, viewport.z, viewport.w);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	static const vec3 up(0, 0, 1);
	const mat4 view = glm::lookAt(getPosition(), getPosition() + getDirection(), up);

	mat4 vp;
	simd_mul(projection, view, vp);

	Program::setBuiltin("MATRIX_VP", vp);
	Program::setBuiltin("cameraPosition", getPosition());


	// Background
	glClearColor(clearColor.r, clearColor.g, clearColor.b, 0.0f);

	glClear(clearFlags);

	Skybox* sky = find<Skybox>();

	if (sky)
		sky->render();

}

/// Setters
void Camera::setRenderingOrder(unsigned _order)
{
	order = _order;
	GraphicEngine::get()->updateCamerasOrder();
}

void Camera::setRenderTexture(RenderTexture* _renderTexture)
{
	delete renderTexture;

	renderTexture = _renderTexture;

	glDeleteFramebuffers(1, &fbo);
	fbo = 0;
	createFramebuffer();
}

void Camera::setClipPlane(vec4 _clipPlane)
{
	clipPlane = _clipPlane;
	Program::setBuiltin("clipPlane", clipPlane);
}

/// Getters
unsigned Camera::getRenderingOrder() const
{
	return order;
}

Texture* Camera::getColorBuffer() const
{
	if (renderTexture)
		return renderTexture->getColorBuffer();
	else
		return nullptr;
}

RenderBuffer* Camera::getDepthBuffer() const
{
	if (renderTexture)
		return renderTexture->getDepthBuffer();
	else
		return nullptr;
}

vec4 Camera::getClipPlane() const
{
	return clipPlane;
}

vec3 Camera::getPosition() const
{
	return tr->getPosition();
}

vec3 Camera::getDirection() const
{
	return tr->getDirection();
}

float Camera::getFOV() const
{
	return FOV;
}

float Camera::getAspectRatio() const
{
	return viewport.z/viewport.w;
}

/// Methods (private)
void Camera::onRegister()
{
	GraphicEngine::get()->addCamera(this);
}

void Camera::onDeregister()
{
	GraphicEngine::get()->removeCamera(this);
}

void Camera::update()
{
	static const vec3 up(0, 0, 1);
	const mat4 view = glm::lookAt(getPosition(), getPosition() + getDirection(), up);

	// Update VP
	simd_mul(projection, view, buckets[0].vp);
}

void Camera::computeViewPort()
{
	vec2 ws;
	if (renderTexture)
		ws = renderTexture->getSize();
	else
		ws = Input::getWindowSize();

	viewport = vec4((int)(relViewport.x * ws.x),
			(int)(relViewport.y * ws.y),
			(int)(relViewport.z * ws.x),
			(int)(relViewport.w * ws.y));

	float aspectRatio = getAspectRatio();

	if (orthographic)
		projection = ortho(-FOV*0.5f, FOV*0.5f, -FOV*0.5f/aspectRatio, FOV*0.5f/aspectRatio, zNear, zFar);
	else
		projection = perspective(FOV, aspectRatio, zNear, zFar);

}

void Camera::createFramebuffer()
{
	if (renderTexture == nullptr)
		return;

	glGenFramebuffersEXT(1, &fbo);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/// Color attachment
		if (renderTexture->getColorBuffer() != nullptr)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture->getColorBuffer()->getId(), 0);

	/// Depth attachement
		if (renderTexture->getDepthBuffer() != nullptr)
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderTexture->getDepthBuffer()->getId());


	int val = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if (val != GL_FRAMEBUFFER_COMPLETE)
		Error::add(OPENGL_ERROR, "Camera::createFramebuffer() -> glCheckFramebufferStatus() returns: " + val);

	if (fbo != 0)
	{
		GLuint buf[1] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, buf);
	}
}

/// Other
vec3 getSymetric(vec3 _point, vec4 _plane)
{
	float k = (_plane.x*_point.x + _plane.y*_point.y + _plane.z*_point.z + _plane.w) / (_plane.x + _plane.y + _plane.z);
	vec3 h(_point - k * vec3(_plane));
	return 2.0f * h - _point;
}
