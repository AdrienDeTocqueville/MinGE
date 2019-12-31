#include "Components/Transform.h"
#include "Components/Light.h"

#include "Systems/GraphicEngine.h"
#include "Renderer/CommandKey.h"

#include "Utility/helpers.h"

#define SHADOW_DIM 512

Light *Light::main = nullptr;

Light::Light(Light::Type _type, vec3 _color, bool _cast_shadows):
	type(_type), color(_color), target(NULL)
{
	if (_cast_shadows)
	{
		target = RenderTarget::create(uvec2(SHADOW_DIM), RenderTarget::DEPTH_32_BIT);
	}
}

Light::~Light()
{ }

/// Methods (public)
Light* Light::clone() const
{
	return new Light(type, color);
}

/// Getters
vec3 Light::getPosition() const
{
	if (type == Light::Directional)
		return tr->getDirection();
	return tr->getPosition();
}

vec3 Light::getColor() const
{
	return color;
}

/// Methods (private)
void Light::onRegister()
{
	GraphicEngine::get()->addLight(this);
}

void Light::onDeregister()
{
	GraphicEngine::get()->removeLight(this);
}

void Light::update(View *view)
{
	vec3 view_pos = getPosition();

	float dim = 10.0f;
	mat4 projection = ortho(-dim, dim, -dim, dim, 1.0f, 7.5f);

	// Compute new VP
	static const vec3 up(0, 0, 1);
	const mat4 view_matrix = glm::lookAt(view_pos, view_pos + tr->getDirection(), up);
	simd_mul(projection, view_matrix, view->vp);

	// Copy data
	view->viewport = ivec4(0, 0, SHADOW_DIM, SHADOW_DIM);
	view->view_pos = view_pos;
	view->clear_flags = GL_DEPTH_BUFFER_BIT;
	view->fbo = target->fbo;

	// Set pass type
	view->pass = RenderPass::Shadow;
}
