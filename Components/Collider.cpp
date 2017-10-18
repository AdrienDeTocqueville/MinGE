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
void Collider::attach(Entity* _entity)
{
    if (entity != nullptr)
        detach();   // Quit actual entity

    entity = _entity;
    tr = entity->getTransform();

    size_t hashCode = typeid(Collider).hash_code();
    entity->components[hashCode].push_back(this);


    rigidBody = getComponent<RigidBody>();
    if (rigidBody != nullptr)
    {
        computeMass();
        rigidBody->computeMass();
    }
}

void Collider::detach()
{
    Component::detach();

    if (rigidBody != nullptr)
        rigidBody->computeMass();

    rigidBody = nullptr;
}

void Collider::registerComponent()
{
    if (registered)
        return;

    PhysicEngine::get()->addCollider(this);
    registered = true;
}

void Collider::deregisterComponent()
{
    if (!registered)
        return;

    PhysicEngine::get()->removeCollider(this);
    registered = false;
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
