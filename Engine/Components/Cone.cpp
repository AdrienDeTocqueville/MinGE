#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/Cone.h"

Cone::Cone(float _radius, float _height, vec3 _center, PhysicMaterialRef _material, bool _isTrigger):
	Collider(_material, _isTrigger, _center),
	radius(_radius), height(_height),
	sinusAngle(radius / sqrt(radius * radius + height * height))
{ }

Cone::~Cone()
{ }

/// Methods (public)
Cone* Cone::clone() const
{
	return new Cone(radius, height, center, material, isTrigger);
}

void Cone::computeMass()
{
	float squareRadius = getRadius(); squareRadius *= squareRadius;
	float h = getHeight();

	float a = (3.0f/20.0f) * squareRadius + (3.0f/5.0f) * h*h;
	float c = (3.0f/10.0f) * squareRadius;

	mass = rigidBody->getDensity() * (1.0f/3.0f) * h * PI * squareRadius;
	inertia = diagonal3x3(mass * vec3(a, a, c));
}

void Cone::computeAABB()
{
	float m = max(0.75f * height, length( vec3(radius, 0.0f, 0.25f * height) ));

	vec3 _center = tr->getToWorldSpace(center);
	vec3 _dim = vec3(m);

	aabb.bounds[0] = _center - _dim;
	aabb.bounds[1] = _center + _dim;
}

/// Getters
vec3 Cone::getSupport(vec3 _axis)
{
	_axis = tr->getVectorToLocalSpace(_axis);

	float axisDistanceSquared = _axis.x * _axis.x + _axis.y * _axis.y;

	vec3 support(0.0f, 0.0f, -height * 0.25f);

	if (_axis.z > sinusAngle)
		support.z = height * 0.75f;

	else if (axisDistanceSquared > EPSILON*EPSILON)
	{
		float rOnDist = radius / sqrt(axisDistanceSquared);
		support = vec3(rOnDist * _axis.x, rOnDist * _axis.y, support.z);
	}

	return tr->getToWorldSpace(support + center);
}

float Cone::getRadius() const
{
	return radius * tr->scale.x;
}

float Cone::getHeight() const
{
	return height * tr->scale.z;
}
