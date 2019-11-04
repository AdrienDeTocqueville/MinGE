#ifndef CONE_H
#define CONE_H

#include "Components/Collider.h"

class Cone : public Collider
{
	public:
		Cone(float _radius = 0.5f, float _height = 1.0f, vec3 _center = vec3(0.0f), PhysicMaterialRef _material = NULL, bool _isTrigger = false);
		virtual ~Cone();

		/// Methods (public)
			virtual Cone* clone() const override;

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

			float sinusAngle;
};

#endif // CONE_H
