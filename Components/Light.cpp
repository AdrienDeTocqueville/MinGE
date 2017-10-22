#include "Components/Transform.h"
#include "Components/Light.h"

#include "Systems/GraphicEngine.h"

Light::Light(LightType _type, vec3 _offset, vec3 _diffuse, float _ambient, float _constant, float _linear, float _quadratic):
    type(_type),
    position(_offset), diffuse(_diffuse), ambient(_ambient),
    attenuation(_constant, _linear, _quadratic)
{ }

Light::~Light()
{ }

/// Methods (public)
Light* Light::clone() const
{
    return new Light(type);
}

/// Getters
vec3 Light::getPosition() const
{
    return tr->getToWorldSpace(position);
}

vec3 Light::getDiffuseColor() const
{
    return diffuse;
}

float Light::getAmbientCoefficient() const
{
    return ambient;
}

vec3 Light::getAttenuation() const
{
    return attenuation;
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
