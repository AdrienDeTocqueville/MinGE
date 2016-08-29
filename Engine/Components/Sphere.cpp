#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/Sphere.h"

Sphere::Sphere(float _radius, vec3 _center, PhysicMaterial* _material, bool _isTrigger):
    Collider(_material, _isTrigger, _center),
    radius(_radius)
{ }

Sphere::~Sphere()
{ }

/// Methods (public)
Component* Sphere::clone()
{
    return new Sphere(radius, center, material, isTrigger);
}

void Sphere::computeMass()
{
    float squareRadius = getRadius();    squareRadius *= squareRadius;

    mass = rigidBody->getDensity() * (4.0f/3.0f) * PI * squareRadius;
    inertia = diagonal3x3(vec3((2.0f/5.0f) * mass * squareRadius));
}

void Sphere::computeAABB()
{
    vec3 r = vec3(radius) * tr->scale;

    aabb.center = tr->getToWorldSpace(center);
    aabb.dim = r;
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
