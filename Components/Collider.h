#ifndef COLLIDER_H
#define COLLIDER_H

#include "Assets/PhysicMaterial.h"
#include "Components/Component.h"
#include "Physic/AABB.h"

struct RayHit
{
    Collider* collider;

    vec3 point;
    vec3 normal;
    float distance;
};

class Collider : public Component
{
    friend class RigidBody;

    public:
        Collider(PhysicMaterial* _material, bool _isTrigger, vec3 _center);
        virtual ~Collider();

        /// Methods (public)
            virtual Collider* clone() const = 0;

            virtual void attach(Entity* _entity) override;
            virtual void detach() override;
            virtual void registerComponent() override;
            virtual void deregisterComponent() override;

            virtual void computeMass() = 0;
            virtual void computeAABB() = 0;

            virtual RayHit raycast(vec3 _o, vec3 _d) { RayHit r; r.distance = -1.0f; return r; }

        /// Getters
            AABB* getAABB();

            float getRestitution() const;
            float getDynamicFriction() const;
            float getStaticFriction() const;

            bool getTrigger() const;
            vec3 getCenter() const;

            virtual vec3 getSupport(vec3 _axis) = 0;

        /// Attributes (public)
            RigidBody* rigidBody;

    protected:
        /// Attributes (protected)
            PhysicMaterial* material;
            bool isTrigger;

            vec3 center;
            float mass;
            mat3 inertia;

            AABB aabb;

};

#endif // COLLIDER_H
