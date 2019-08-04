#ifndef BOX_H
#define BOX_H

#include "Components/Collider.h"

class Box : public Collider
{
    public:
        Box(vec3 _halfExtent = vec3(0.5f), vec3 _center = vec3(0.0f), PhysicMaterial* _material = PhysicMaterial::base, bool _isTrigger = false);
        virtual ~Box();

        /// Methods (public)
            virtual Box* clone() const override;

            virtual void computeMass() override;
            virtual void computeAABB() override;

            virtual RayHit raycast(vec3 _o, vec3 _d) override;

        /// Setters
            void setHalfExtent(vec3 _halfExtent);

        /// Getters
            virtual vec3 getSupport(vec3 _axis) override;

            vec3 getHalfExtent() const;
            vec3 getDimensions() const;

    private:
        /// Attributes
            vec3 halfExtent;
};

#endif // BOX_H
