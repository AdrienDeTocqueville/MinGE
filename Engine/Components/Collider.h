#ifndef COLLIDER_H
#define COLLIDER_H

#include "Assets/PhysicMaterial.h"
#include "Components/Component.h"
#include "Utility/AABB.h"

struct RayHit
{
	Collider* collider = nullptr;

	vec3 point;
	vec3 normal = vec3(0.0f);
	float distance = -1.0f;
};

class Collider : public Component
{
	friend class Entity;
	friend class RigidBody;

	public:
		Collider(PhysicMaterial* _material, bool _isTrigger, vec3 _center);
		virtual ~Collider();

		/// Methods (public)
			virtual Collider* clone() const = 0;

			virtual void computeMass() = 0;
			virtual void computeAABB() = 0;

			// TODO: make it virtual pure
			virtual RayHit raycast(vec3 _o, vec3 _d);

		/// Getters
			AABB* getAABB();

			float getRestitution() const;
			float getDynamicFriction() const;
			float getStaticFriction() const;

			bool getTrigger() const;
			vec3 getCenter() const;

			virtual vec3 getSupport(vec3 _axis) = 0;

		// TODO: make it private
		/// Attributes (public)
			RigidBody* rigidBody;

	protected:
		/// Methods (private)
			virtual void onRegister() override;
			virtual void onDeregister() override;

		/// Attributes (protected)
			PhysicMaterial* material;
			bool isTrigger;

			vec3 center;
			float mass;
			mat3 inertia;

			AABB aabb;

};

#endif // COLLIDER_H
