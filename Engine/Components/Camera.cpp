#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"
#include "Renderer/CommandKey.h"

#include "Assets/Program.h"

Camera::Camera(float _FOV, float _zNear, float _zFar, vec3 _clearColor, RenderTargetRef _target, bool _orthographic, vec4 _viewport, unsigned _clearFlags):
	FOV(_FOV), zNear(_zNear), zFar(_zFar),
	clearColor(_clearColor), clearFlags(_clearFlags),
	orthographic(_orthographic),
	relViewport(_viewport),
	renderTarget(_target ? _target : RenderTarget::getDefault())
{
	computeViewPort();
}

/// Methods (public)
Camera* Camera::clone() const
{
	return new Camera(FOV, zNear, zFar, clearColor, renderTarget, orthographic, relViewport, clearFlags);
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
	return (float)viewport.z / (float)viewport.w;
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

void Camera::update(View *view)
{
	// Compute new VP
	static const vec3 up(0, 0, 1);
	const mat4 view_matrix = glm::lookAt(getPosition(), getPosition() + getDirection(), up);
	simd_mul(projection, view_matrix, view->vp);

	// Copy data
	view->viewport = viewport;
	view->clearColor = vec4(clearColor, 0.0f);
	view->clearFlags = clearFlags;
	view->fbo = renderTarget->fbo;

	// Set pass type
	view->pass = RenderPass::Forward;

	// TODO: find a solution for that
	Program::setBuiltin("cameraPosition", getPosition());
}

void Camera::computeViewPort()
{
	vec2 ws = renderTarget->getSize();

	viewport = ivec4(relViewport.x * ws.x,
		relViewport.y * ws.y,
		relViewport.z * ws.x,
		relViewport.w * ws.y
	);

	float aspectRatio = getAspectRatio();

	if (orthographic)
		projection = ortho(-FOV*0.5f, FOV*0.5f, -FOV*0.5f/aspectRatio, FOV*0.5f/aspectRatio, zNear, zFar);
	else
		projection = perspective(FOV, aspectRatio, zNear, zFar);

}
