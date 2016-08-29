#include "Components\Transform.h"
#include "Components\RigidBody.h"
#include "Components\Collider.h"

#include "Systems/PhysicEngine.h"

RigidBody::RigidBody(float _density):
    COM(vec3(0.0f)),
    forces(0.0f), torques(0.0f),
    linearVelocity(0.0f), angularVelocity(0.0f),
    density(max(_density, 0.0f)),
    mass(0.0f), iM(0.0f),
    inertia(mat3(0.0f)), iI(mat3(0.0f)), iILocal(mat3(0.0f)),
    linearDamping(0.25f), angularDamping(0.25f)
{ }

RigidBody::~RigidBody()
{ }

/// Methods (public)
Component* RigidBody::clone()
{
    return new RigidBody(density);
}

void RigidBody::attach(Entity* _entity)
{
    Component::attach(_entity);

    for (Collider* collider: getComponents<Collider>())
        collider->rigidBody = this;

    float save = density; density = -1.0f;

    setDensity(save);
}

void RigidBody::detach()
{
    if (entity == nullptr)
        return;

    for (Collider* collider: getComponents<Collider>())
        collider->rigidBody = nullptr;

    Component::detach();
}

void RigidBody::registerComponent()
{
    PhysicEngine::get()->addRigidBody(this);
}

void RigidBody::deregisterComponent()
{
    PhysicEngine::get()->removeRigidBody(this);
}

void RigidBody::computeMass()
{
    COM = vec3(0.0f);
    mass = iM = 0.0f;
    inertia = iILocal = iI = mat3(0.0f);

    if (density == 0.0f)
        return;

    // Mass and COM
        for (Collider* collider: getComponents<Collider>())
        {
            mass += collider->mass;
            COM += collider->mass * collider->center * tr->scale;
        }

        if (mass)
        {
            iM = 1.0f/mass;
            COM *= iM;
        }

    // Inertia
        for (Collider* collider: getComponents<Collider>())
        {
            const vec3 r = COM - collider->center * tr->scale;
            const float d = dot(r, r);
            const mat3  o = outerProduct(r, r);

            inertia += collider->inertia + collider->mass* (d*mat3(1.0f) - o);
        }

        if (inertia != mat3(0.0f))
            iILocal = inverse(inertia);

        iI = tr->getToWorldSpace(iILocal);
}

void RigidBody::integrateForces(float _dt)
{
    if (!iM)
        return;

    linearVelocity  += (iM * forces ) * _dt;
    angularVelocity += (iI * torques) * _dt;

    forces  = vec3(0.0f);
    torques = vec3(0.0f);

    // Equation: F = -c * v  =>  dv/dt + c * v = 0
    // Solution: v(t) = v0 * exp(-c * t)
    // Time step: v(t + dt) = v0 * exp(-c * (t + dt)) = v0 * exp(-c * t) * exp(-c * dt) = v * exp(-c * dt)
    // v2 = exp(-c * dt) * v1
    // Pade approximation:
    // v2 = v1 * 1 / (1 + c * dt)
    linearVelocity *= 1.0f / (1.0f + _dt * linearDamping);
	angularVelocity *= 1.0f / (1.0f + _dt * angularDamping);

    for (unsigned i(0) ; i < 3 ; i++)
    {
        if (isnan(linearVelocity[i]))
            linearVelocity[i] = 0.0f;

        if (isnan(angularVelocity[i]))
            angularVelocity[i] = 0.0f;
    }
}

void RigidBody::integrateVelocities(float _dt)
{
    if (!iM)
        return;

    for (unsigned i(0) ; i < 3 ; i++)
    {
        if (isnan(linearVelocity[i]))
            linearVelocity[i] = 0.0f;

        if (isnan(angularVelocity[i]))
            angularVelocity[i] = 0.0f;
    }

    // Integrate linear velocity
        tr->position += linearVelocity * _dt;

    // Integrate angular velocity
        tr->rotation += 0.5f *_dt * quat(0.0f, angularVelocity) * tr->rotation;
        if (tr->rotation != quat(0.0f, 0.0f, 0.0f, 0.0f))
            tr->rotation = normalize(tr->rotation);

    // Update transformation matrices
        tr->toMatrix();


    // Compute inertia matrix in world space
        iI = tr->getToWorldSpace(iILocal);
}

void RigidBody::applyImpulse(vec3 _J0, vec3 _J1, float _lambda)
{
    linearVelocity  += iM * _lambda * _J0;
    angularVelocity += iI * _lambda * _J1;
}

void RigidBody::applyForce(vec3 _force, vec3 _point)
{
    forces += _force;
    torques += cross(_point - getCOM(), _force);

    Debug::drawVector(getCOM(), _point, vec3(249, 54, 13)*(1.0f/255.0f));
}

void RigidBody::applyForceToCOM(vec3 _force)
{
    forces += _force;
}

void RigidBody::updateAABBs()
{
    for (Collider* collider: getComponents<Collider>())
    {
        collider->computeAABB();
        collider->getAABB()->prepare();
    }
}

/// Setter
void RigidBody::setLinearVelocity(vec3 _velocity)
{
    linearVelocity = _velocity;
}

void RigidBody::setAngularVelocity(vec3 _velocity)
{
    angularVelocity = _velocity;
}

void RigidBody::setDensity(float _density)
{
    if (density == _density || _density < 0.0f)
        return;

    density = _density;

    for (Collider* collider: getComponents<Collider>())
        collider->computeMass();

    computeMass();
}

/// Getter
vec3 RigidBody::getCOM() const
{
    return tr->getToWorldSpace(COM);
}

vec3 RigidBody::getLinearVelocity() const
{
    return linearVelocity;
}

vec3 RigidBody::getAngularVelocity() const
{
    return angularVelocity;
}

float RigidBody::getMass() const
{
    return mass;
}

float RigidBody::getDensity() const
{
    return density;
}
