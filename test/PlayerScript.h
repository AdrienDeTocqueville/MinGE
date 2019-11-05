#pragma once

#include <MinGE.h>

class PlayerScript : public Script
{
	public:
		PlayerScript(float _speed, float _alt_speed);

		/// Methods (public)
			void start() override
			{
				rb = find<RigidBody>();
				cam = Entity::findByTag("MainCamera", false)->find<Camera>();
			}

			void update() override;

	private:
		RigidBody* rb;
		Camera *cam;

		float speed, alt_speed;
};
