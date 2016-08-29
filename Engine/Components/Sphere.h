#ifndef SPHERE_H
#define SPHERE_H

#include "Components/Collider.h"

class Sphere : public Collider
{
    public:
        Sphere(float _radius = 0.5f, vec3 _center = vec3(0.0f), PhysicMaterial* _material = PhysicMaterial::base, bool _isTrigger = false);
        virtual ~Sphere();

        /// Methods (public)
            virtual Component* clone() override;

            virtual void computeMass() override;
            virtual void computeAABB() override;

        /// Getters
            virtual vec3 getSupport(vec3 _axis) override;

            float getRadius();

        /// Attributes
            float radius;
};

#endif // SPHERE_H
