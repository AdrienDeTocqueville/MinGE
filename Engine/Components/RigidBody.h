#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "Components/Component.h"

class RigidBody: public Component
{
    friend class Entity;

    friend class FixedConstraint;
    friend class ContactConstraint;
    friend class DistanceConstraint;

    public:
        RigidBody(float _density = 1.0f);
        virtual ~RigidBody();

        /// Methods (public)
            virtual RigidBody* clone() const override;

            void computeMass();

            void integrateForces(float _dt);
            void integrateVelocities(float _dt);

            void applyImpulse(vec3 _J0, vec3 _J1, float _lambda);

            void applyForce     (vec3 _force, vec3 _point);
            void applyForceToCOM(vec3 _force);

        /// Setter
            void setLinearVelocity(vec3 _velocity);
            void setAngularVelocity(vec3 _velocity);

            void setDensity(float _density);

        /// Getter
            vec3 getCOM() const;

            vec3 getLinearVelocity() const;
            vec3 getAngularVelocity() const;

            float getMass() const;
            float getDensity() const;

    private:
        /// Methods (private)
            virtual void onRegister() override;
            virtual void onDeregister() override;

        /// Attributes (private)
            vec3 COM;

            vec3 forces, torques;
            vec3 linearVelocity, angularVelocity;

            float density;
            float mass, iM;
            mat3 inertia, iI, iILocal;

            float linearDamping;
            float angularDamping;
};

#endif // RIGIDBODY_H
