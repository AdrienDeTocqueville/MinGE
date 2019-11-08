#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Transform.h"

#include "Assets/Program.h"
#include "Systems/GraphicEngine.h"
#include "Renderer/CommandBucket.h"

Camera::Camera(float _FOV, float _zNear, float _zFar, vec3 _clearColor, RenderTargetRef _target, bool _orthographic, vec4 _viewport, unsigned _clearFlags):
	FOV(_FOV), zNear(_zNear), zFar(_zFar),
	clearColor(_clearColor), clearFlags(_clearFlags),
	orthographic(_orthographic),
	relViewport(_viewport),
	renderTarget(_target ? _target : RenderTarget::getDefault())
{
	computeViewPort();

	/*
	buckets.push_back({
		viewport, renderTarget, mat4(1.0f),
		vec4(clearColor, 0.0f), clearFlags,
		RenderPass::Forward
	});
	*/
}

/// Methods (public)
Camera* Camera::clone() const
{
	return new Camera(FOV, zNear, zFar, clearColor, renderTarget, orthographic, relViewport, clearFlags);
}

void Camera::use()
{
	GL::Viewport(viewport);
	GL::Scissor (viewport);

	GL::BindFramebuffer(renderTarget->fbo);


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

/// Getters
RenderTargetRef Camera::getRenderTarget() const
{
	return renderTarget;
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
	//simd_mul(projection, view, buckets[0].vp);

	//Program::setBuiltin("MATRIX_VP", vp);
	//Program::setBuiltin("cameraPosition", getPosition());
}

void Camera::computeViewPort()
{
	vec2 ws = renderTarget->getSize();

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
