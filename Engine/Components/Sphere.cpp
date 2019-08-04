#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/Sphere.h"

#include <glm/gtx/matrix_operation.hpp>

Sphere::Sphere(float _radius, vec3 _center, PhysicMaterial* _material, bool _isTrigger):
	Collider(_material, _isTrigger, _center),
	radius(_radius)
{ }

Sphere::~Sphere()
{ }

/// Methods (public)
Sphere* Sphere::clone() const
{
	return new Sphere(radius, center, material, isTrigger);
}

void Sphere::computeMass()
{
	float squareRadius = getRadius();	squareRadius *= squareRadius;

	mass = rigidBody->getDensity() * (4.0f/3.0f) * PI * squareRadius;
	inertia = diagonal3x3(vec3((2.0f/5.0f) * mass * squareRadius));
}

void Sphere::computeAABB()
{
	vec3 r = vec3(radius) * tr->scale;

	aabb.center = tr->getToWorldSpace(center);
	aabb.dim = r;
}

RayHit Sphere::raycast(vec3 _o, vec3 _d)
{
	RayHit r;

	vec3 OC = tr->position - _o;

	if (dot(OC, _d) < 0)
		return r;

	float rad = getRadius();
	if (!epsilonEqual(length2(_d), 1.0f, EPSILON)) // _d must be unit length
		_d = normalize(_d);

	float b = 2.0f* (OC.x*_d.x + OC.y*_d.y + OC.z*_d.z);
	float c = length2(OC) - rad*rad;

	float delta = b*b - 4.0f*c;

	if (delta < 0.0f)
		return r;

	float m = (b - sqrt(delta)) * 0.5f;

	if (m > 0)
	{
		r.collider = this;
		r.point = _o + m*_d;
		r.normal = (r.point - tr->position) / radius;
		r.distance = m;
	}

	return r;
}

/// Getters
vec3 Sphere::getSupport(vec3 _axis)
{
	return tr->position + (center + radius * _axis) * tr->scale;
}

float Sphere::getRadius()
{
	return radius * tr->scale.x;
}
