#ifndef CYLINDER_H
#define CYLINDER_H

#include "Components/Collider.h"

class Cylinder : public Collider
{
	public:
		Cylinder(float _radius = 0.5f, float _height = 1.0f, vec3 _center = vec3(0.0f), PhysicMaterial* _material = PhysicMaterial::base, bool _isTrigger = false);
		virtual ~Cylinder();

		/// Methods (public)
			virtual Cylinder* clone() const override;

			virtual void computeMass() override;
			virtual void computeAABB() override;

		/// Getters
			virtual vec3 getSupport(vec3 _axis) override;

			float getRadius() const;
			float getHeight() const;

	private:
		/// Attributes
			float radius;
			float height;
};

#endif // CYLINDER_H
