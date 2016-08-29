#include "Systems/PhysicEngine.h"

#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/Collider.h"

#include "Physic/Constraint.h"
#include "Physic/DistanceConstraint.h"
#include "Physic/ContactConstraint.h"


PhysicEngine* PhysicEngine::instance = nullptr;

/// Methods (private)
PhysicEngine::PhysicEngine(vec3 _gravity):
    maxIterations(15),
    accumulator(0.0f), dt(1.0f / 60.0f)
{
    Dispatcher::fill();
    setGravity(_gravity);
}

PhysicEngine::~PhysicEngine()
{
    Dispatcher::clear();
}

void PhysicEngine::clear()
{
	bodies.clear();
	colliders.clear();
	constraints.clear();

	DistanceConstraint::clear();
}

/// Methods (static)
void PhysicEngine::create(vec3 _gravity)
{
    if (instance != nullptr)
        return;

    instance = new PhysicEngine(_gravity);
}

void PhysicEngine::destroy()
{
    delete instance;
    instance = nullptr;
}

PhysicEngine* PhysicEngine::get()
{
    return instance;
}

/// Methods (public)
void PhysicEngine::addRigidBody(RigidBody* _body)
{
    if (_body != nullptr)
        bodies.push_back(_body);
}

void PhysicEngine::addCollider(Collider* _collider)
{
    if (_collider != nullptr)
    {
        _collider->computeAABB();
        colliders.push_back(_collider);
    }
}

void PhysicEngine::addConstraint(Constraint* _constraint)
{
    if (_constraint != nullptr)
        constraints.push_back(_constraint);
}

void PhysicEngine::removeRigidBody(RigidBody* _body)
{
    bodies.remove(_body);
}

void PhysicEngine::removeCollider(Collider* _collider)
{
    colliders.remove(_collider);
}

void PhysicEngine::removeConstraint(Constraint* _constraint)
{
    constraints.remove(_constraint);
}

void PhysicEngine::simulate()
{
    accumulator += std::min(Component::deltaTime, 0.2f);

    // main loop
    while(accumulator > dt)
    {
        update();
        accumulator -= dt;
    }
}

void PhysicEngine::update()
{
    // Generate collision informations
    for(auto i(colliders.begin()) ; i != colliders.end() ; ++i)
        for(auto j(std::next(i)) ; j != colliders.end() ; ++j)
            detectCollision(*i, *j);

    // Detect active constraints
    for(Constraint* constraint: constraints)
    {
        if (constraint->positionConstraint())
            activeConstraints.push_back(constraint);
    }

    // Integrate forces
    for(RigidBody* body: bodies)
    {
        body->applyForceToCOM(body->getMass() * gravity);

        body->integrateForces(dt);
    }

    // Solve contacts with sequential impulses
    for(unsigned j = 0; j < maxIterations; ++j)
    {
        for(ContactConstraint* collision: collisions)
            collision->velocityConstraint(dt);

        for(Constraint* constraint: activeConstraints)
            constraint->velocityConstraint(dt);
    }

    // Integrate velocities
    for(RigidBody* body: bodies)
    {
        body->integrateVelocities(dt);
        body->updateAABBs();
    }


    sendAndFreeData();
}

void PhysicEngine::detectCollision(Collider* a, Collider* b)
{
    if (a->getTransform()->getRoot() == b->getTransform()->getRoot())
        return;

    if (a->rigidBody && b->rigidBody)
    {
        if (!a->rigidBody->getMass() && !b->rigidBody->getMass())
            return;
    }

    if (!AABB::collide( a->getAABB(), b->getAABB() ))
        return;



    ContactConstraint* contact = new ContactConstraint(a, b);

    if (!contact->positionConstraint())    // not colliding
        delete contact;
    else
    {
        if (contact->type == 0)
            collisions.push_back(contact);
        else
            delete contact;
//            triggers.push_back(contact);
    }
}

void PhysicEngine::sendAndFreeData()
{
    activeConstraints.clear();

    // Send to scripts and clear lists
    for(ContactConstraint* trigger: triggers)
    {
        trigger->sendData();
        delete trigger;
    }
    triggers.clear();

    for(ContactConstraint* collision: collisions)
    {
        collision->sendData();
        delete collision;
    }
    collisions.clear();
}

void PhysicEngine::setGravity(vec3 _gravity)
{
    gravity = _gravity;
    gravityValue = length(gravity);
}
