#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/Cylinder.h"

#include <glm/gtx/matrix_operation.hpp>

Cylinder::Cylinder(float _radius, float _height, vec3 _center, PhysicMaterial* _material, bool _isTrigger):
	Collider(_material, _isTrigger, _center),
	radius(_radius), height(_height)
{ }

Cylinder::~Cylinder()
{ }

/// Methods (public)
Cylinder* Cylinder::clone() const
{
	return new Cylinder(radius, height, center, material, isTrigger);
}

void Cylinder::computeMass()
{
	float squareRadius = getRadius(); squareRadius *= squareRadius;
	float h = getHeight();

	float a = 0.25f * squareRadius + (1.0f/12.0f) * h * h;
	float c = 0.5f * squareRadius;

	mass = rigidBody->getDensity() * h * PI * squareRadius;
	inertia = diagonal3x3(mass * vec3(a, a, c));
}

void Cylinder::computeAABB()
{
	float m = length( vec3(getRadius(), 0.0f, 0.5f * getHeight()) );

	vec3 _center = tr->getToWorldSpace(center);
	vec3 _dim = vec3(m);

	aabb.bounds[0] = _center - _dim;
	aabb.bounds[1] = _center + _dim;
}

/// Getters
vec3 Cylinder::getSupport(vec3 _axis)
{
	_axis = tr->getVectorToLocalSpace(_axis);

	float halfHeight = height * 0.5f;
	float axisDistanceSquared = _axis.x * _axis.x + _axis.y * _axis.y;

	float sign = (0.0f < _axis.z) - (_axis.z < 0.0f);   // get sign of _axis.z

	vec3 support(0.0f, 0.0f, sign * halfHeight);

	if (axisDistanceSquared > EPSILON*EPSILON)
	{
		float rOnDist = radius / sqrt(axisDistanceSquared);
		support =  vec3(rOnDist * _axis.x , rOnDist * _axis.y, support.z);
	}

	return tr->getToWorldSpace(support + center);
}

float Cylinder::getRadius() const
{
	return radius * tr->scale.x;
}

float Cylinder::getHeight() const
{
	return height * tr->scale.z;
}
