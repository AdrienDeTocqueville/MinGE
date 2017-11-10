#include "Components/RigidBody.h"
#include "Components/Collider.h"

#include "Systems/PhysicEngine.h"


Collider::Collider(PhysicMaterial* _material, bool _isTrigger, vec3 _center):
    rigidBody(nullptr),
    material(_material), isTrigger(_isTrigger),
    center(_center), mass(0.0f), inertia(1.0f)
{ }

Collider::~Collider()
{ }

/// Methods (public)
RayHit Collider::raycast(vec3 _o, vec3 _d)
{
    return RayHit();
}

/// Getters
AABB* Collider::getAABB()
{
    return &aabb;
}

float Collider::getRestitution() const
{
    return material->restitution;
}

float Collider::getDynamicFriction() const
{
    return material->dynamicFriction;
}

float Collider::getStaticFriction() const
{
    return material->staticFriction;
}

bool Collider::getTrigger() const
{
    if (rigidBody == nullptr)
        return true;

    return isTrigger;
}

vec3 Collider::getCenter() const
{
    return center;
}

/// Methods (private)
void Collider::onRegister()
{
    rigidBody = find<RigidBody>();
    if (rigidBody != nullptr)
    {
        computeMass();
        rigidBody->computeMass();
    }

    PhysicEngine::get()->addCollider(this);
}

void Collider::onDeregister()
{
    if (rigidBody != nullptr)
        rigidBody->computeMass();

    PhysicEngine::get()->removeCollider(this);

    rigidBody = nullptr;
}
