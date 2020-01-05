#include "Components/Transform.h"
#include "Components/Light.h"

#include "Systems/GraphicEngine.h"
#include "Renderer/CommandKey.h"

#include "Assets/Shader.h"
#include "Assets/Shader.inl"

#include "Utility/helpers.h"

#define SHADOW_DIM 512

Light *Light::main = nullptr;
Light *Light::bound = nullptr;

Light::Light(Light::Type _type, vec3 _color, bool _cast_shadow):
	type(_type), color(_color), cast_shadow(_cast_shadow), target(NULL)
{ }

Light::~Light()
{ }

/// Methods (public)
Light* Light::clone() const
{
	return new Light(type, color, cast_shadow);
}

void Light::bind() const
{
	if (Light::bound == this)
		return;
	Light::bound == this;

	if (cast_shadow)
	{
		Shader::setBuiltin("MATRIX_LIGHT", light_space);
		Shader::setBuiltin("SHADOW_MAP", target->getColorBuffer());
	}
	Shader::setBuiltin("LIGHT_DIR", tr->vectorToWorld(vec3(0, 0, 1)));
	Shader::setBuiltin("LIGHT_COLOR", color);
}

/// Getters
vec3 Light::getPosition() const
{
	return tr->getPosition();
}

vec3 Light::getDirection() const
{
	return tr->vectorToWorld(vec3(0, 0, -1));
}

/// Methods (private)
void Light::onRegister()
{
	if (cast_shadow && target == NULL)
		target = std::move(RenderTarget::create_depth_map(uvec2(SHADOW_DIM)));

	GraphicEngine::get()->addLight(this);
}

void Light::onDeregister()
{
	GraphicEngine::get()->removeLight(this);
}

void Light::update(View *view)
{
	const vec3 view_pos = getPosition();
	const vec3 view_dir = getDirection();

	static const vec3 up(0, 0, 1);
	const float dim = 10.0f;
	const mat4 projection = ortho(-dim, dim, -dim, dim, 1.0f, 20.0f);
	const mat4 view_matrix = glm::lookAt(view_pos, view_pos + view_dir, up);
	simd_mul(projection, view_matrix, light_space);

	// Copy data
	view->vp = light_space;
	view->viewport = ivec4(0, 0, SHADOW_DIM, SHADOW_DIM);
	view->view_pos = view_pos;
	view->clear_flags = GL_DEPTH_BUFFER_BIT;
	view->fbo = target->fbo;

	// Set pass type
	view->pass = RenderPass::Shadow;
}
