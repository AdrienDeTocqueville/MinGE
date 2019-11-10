#include "Components/Transform.h"
#include "Components/Light.h"

#include "Systems/GraphicEngine.h"

Light::Light(LightType _type, vec3 _offset, vec3 _color):
	type(_type), offset(_offset), color(_color)
{ }

Light::~Light()
{ }

/// Methods (public)
Light* Light::clone() const
{
	return new Light(type, offset, color);
}

/// Getters
vec3 Light::getPosition() const
{
	return tr->toWorld(offset);
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
