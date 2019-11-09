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
	CommandBucket *bucket = &renderTarget->bucket;

	unsigned view_id= bucket->add_view();
	CommandBucket::View *view = bucket->get_view(view_id);

	view->passes = (1<<RenderPass::Forward);// | (1<<RenderPass::Additive);

	view->viewport = viewport;
	view->clearColor = vec4(clearColor, 0.0f);
	view->clearFlags = clearFlags;

	// Compute new VP
	static const vec3 up(0, 0, 1);
	const mat4 view_matrix = glm::lookAt(getPosition(), getPosition() + getDirection(), up);
	simd_mul(projection, view_matrix, view->vp);

	uint64_t key = CommandKey::encode(view_id, RenderPass::Forward);
	SetupView *setup = bucket->add<SetupView>(key);
	setup->view = view;


	if (Skybox* sky = find<Skybox>())
		sky->render();

	// TODO: find a solution for that
	Program::setBuiltin("cameraPosition", getPosition());
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
