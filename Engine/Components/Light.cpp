#include "Components/Transform.h"
#include "Components/Light.h"

#include "Systems/GraphicEngine.h"

Light::Light(Light::Type _type, vec3 _color):
	type(_type), color(_color)
{ }

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
