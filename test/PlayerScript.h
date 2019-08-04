#pragma once

#include <MinGE.h>

class PlayerScript : public Script
{
	public:
		PlayerScript(float _speed, float _minSpeed, float _maxSpeed);

		/// Methods (public)
			void start() override
			{
				rb = find<RigidBody>();
			}

			void update() override;

	private:
		RigidBody* rb;

		float speed;
		float minSpeed, maxSpeed;
};
