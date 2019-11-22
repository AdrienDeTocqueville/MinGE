#ifndef PHYSICENGINE_H
#define PHYSICENGINE_H

#include "Utility/helpers.h"

class RigidBody;
class Collider;

class RayHit;
class Constraint;
class ContactConstraint;

class PhysicEngine
{
	friend class Engine;

	public:
		/// Methods (static)
			static PhysicEngine* get();

		/// Methods (public)
			void addRigidBody(RigidBody* _body);
			void addCollider(Collider* _collider);
			void addConstraint(Constraint* _constraint);

			void removeRigidBody(RigidBody* _body);
			void removeCollider(Collider* _collider);
			void removeConstraint(Constraint* _constraint);

			void simulate();
			void update();

			RayHit raycast(vec3 _origin, vec3 _direction);
			std::vector<RayHit> raycastAll(vec3 _origin, vec3 _direction, bool _sort = false);

			void setGravity(vec3 _gravity = vec3(0, 0, -9.81f));

	private:
		/// Methods (private)
			PhysicEngine(vec3 _gravity);
			~PhysicEngine();

			void detectCollision(Collider* a, Collider* b);
			void sendAndFreeData();

			void clear();

			static void create(vec3 _gravity = vec3(0, 0, -9.81f));
			static void destroy();

		/// Attributes (private)
			std::vector<RigidBody* > bodies;
			std::vector<Collider*  > colliders;
			std::vector<Constraint*> constraints;

			std::vector<Constraint*> activeConstraints;
			std::vector<ContactConstraint*> triggers;
			std::vector<ContactConstraint*> collisions;

			vec3 gravity;
			float gravityValue;

			unsigned maxIterations;
			float accumulator;
			float dt;

		/// Attributes (static)
			static PhysicEngine* instance;
};

#endif // PHYSICENGINE_H
