#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"
#include "Renderer/CommandKey.h"

#include "Assets/Shader.h"
#include "Assets/Shader.inl"

Camera::Camera(float _FOV, float _zNear, float _zFar, vec3 _clearColor, RenderTargetRef _target, bool _orthographic,
	vec4 _viewport, unsigned _clearFlags, unsigned _priority):
	FOV(_FOV), zNear(_zNear), zFar(_zFar), priority(_priority),
	clearColor(_clearColor), clearFlags(_clearFlags),
	orthographic(_orthographic), relViewport(_viewport),
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
	vec3 view_pos = getPosition();

	// Compute new VP
	static const vec3 up(0, 0, 1);
	const mat4 view_matrix = glm::lookAt(view_pos, view_pos + getDirection(), up);
	simd_mul(projection, view_matrix, view->vp);

	// Copy data
	view->viewport = viewport;
	view->clear_color = vec4(clearColor, 0.0f);
	view->view_pos = view_pos;
	view->clear_flags = clearFlags;
	view->fbo = renderTarget->fbo;

	// Set pass type
	view->pass = RenderPass::Forward;
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
