#pragma once

#include <MinGE.h>


void test_physic();

class TestPhysic : public Script
{
	public:
		TestPhysic(Entity* _prot);

		/// Methods (public)
			void start() override;
			void update() override;

			void setShape(int shape)
			{
				if (shape == current)
					return;

				entity->find<Graphic>()->setMesh(shapes[shape]);
				entity->remove<Collider>();
				if (shape == 0) entity->insert<Box>();
				if (shape == 1) entity->insert<Cylinder>();
				if (shape == 2) entity->insert<Cone>();
				if (shape == 3) entity->insert<Sphere>();

				current = shape;
			}

	private:
		Entity* prot;

		float fireRate = 0.75f, time = 0.0f;

		DistanceConstraint* dj;

		Mesh* shapes[4];
		int current;
};
