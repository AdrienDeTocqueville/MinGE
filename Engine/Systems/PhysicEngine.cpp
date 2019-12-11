#include "Systems/PhysicEngine.h"

#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/Collider.h"

#include "Physic/Constraint.h"
#include "Physic/DistanceConstraint.h"
#include "Physic/ContactConstraint.h"

#include "Utility/Time.h"

#include "Profiler/profiler.h"

bool sortDistance(const RayHit& _a, const RayHit& _b);

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
	auto it = std::find(bodies.begin(), bodies.end(), _body);
	if (it != bodies.end())
	{
		*it = bodies.back();
		bodies.pop_back();
	}
}

void PhysicEngine::removeCollider(Collider* _collider)
{
	auto it = std::find(colliders.begin(), colliders.end(), _collider);
	if (it != colliders.end())
	{
		*it = colliders.back();
		colliders.pop_back();
	}
}

void PhysicEngine::removeConstraint(Constraint* _constraint)
{
	auto it = std::find(constraints.begin(), constraints.end(), _constraint);
	if (it != constraints.end())
	{
		*it = constraints.back();
		constraints.pop_back();
	}
}

void PhysicEngine::simulate()
{
	MICROPROFILE_SCOPEI("SYSTEM_PHYSIC", "simulate");

	accumulator += Time::deltaTime;

	// main loop
	while(accumulator > dt)
	{
		update();
		accumulator -= dt;
	}
}

void PhysicEngine::update()
{
	MICROPROFILE_SCOPEI("SYSTEM_PHYSIC", "update");

	// Generate collision informations
	for (auto i(colliders.begin()) ; i != colliders.end() ; ++i)
		for (auto j(std::next(i)) ; j != colliders.end() ; ++j)
			detectCollision(*i, *j);

	// Detect active constraints
	for (Constraint* constraint: constraints)
	{
		if (constraint->positionConstraint())
			activeConstraints.push_back(constraint);
	}

	// Integrate forces
	for (RigidBody* body: bodies)
	{
		body->applyForceToCOM(body->getMass() * gravity);

		body->integrateForces(dt);
	}

	// Solve contacts with sequential impulses
	for (unsigned j = 0; j < maxIterations; ++j)
	{
		for (ContactConstraint* collision: collisions)
			collision->velocityConstraint(dt);

		for (Constraint* constraint: activeConstraints)
			constraint->velocityConstraint(dt);
	}

	// Integrate velocities
	for (RigidBody* body: bodies)
	{
		body->integrateVelocities(dt);
	}

	for (Collider* collider: colliders)
	{
		collider->computeAABB();
#ifdef DRAWAABB
		collider->getAABB()->prepare();
#endif
	}

	sendAndFreeData();
}

RayHit PhysicEngine::raycast(vec3 _origin, vec3 _direction)
{
	RayHit closestHit;

	for (Collider* collider: colliders)
	{
		RayHit hit = collider->raycast(_origin, _direction);

		if (hit.distance != -1.0f)
			if (closestHit.distance == -1.0f || hit.distance < closestHit.distance)
				closestHit = hit;
	}

	return closestHit;
}

bool sortDistance(const RayHit& _a, const RayHit& _b) { return _a.distance < _b.distance; }

std::vector<RayHit> PhysicEngine::raycastAll(vec3 _origin, vec3 _direction, bool _sort)
{
	std::vector<RayHit> hits;

	for (Collider* collider: colliders)
	{
		RayHit hit = collider->raycast(_origin, _direction);

		if (hit.distance != -1.0f)
			hits.push_back(hit);
	}


	if (_sort)
		std::sort(hits.begin(), hits.end(), sortDistance);

	return hits;
}

void PhysicEngine::detectCollision(Collider* a, Collider* b)
{
	if (a->find<Transform>()->getRoot() == b->find<Transform>()->getRoot())
		return;

	if (a->rigidBody && b->rigidBody)
	{
		if (!a->rigidBody->getMass() && !b->rigidBody->getMass())
			return;
	}

	if (!AABB::collide( a->getAABB(), b->getAABB() ))
		return;


	ContactConstraint* contact = new ContactConstraint(a, b);

	if (!contact->positionConstraint())	// not colliding
		delete contact;
	else
	{
		if (contact->type == 0)
			collisions.push_back(contact);
		else
			delete contact;
//			triggers.push_back(contact);
	}
}

void PhysicEngine::sendAndFreeData()
{
	activeConstraints.clear();

	// Send to scripts and clear arrays
	for (ContactConstraint* trigger: triggers)
	{
		trigger->sendData();
		delete trigger;
	}
	triggers.clear();

	for (ContactConstraint* collision: collisions)
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
