#pragma once

#include <MinGE.h>

class CameraScript : public Script
{
	public:
		CameraScript(Transform* _target, float _sensivity = 0.2f, float _distance = 0.01f, vec3 _offset = vec3(0.0f)):
			target(_target), angles(0.0f), clampAngleY(-0.499f*PI, 0.499f*PI),
			sensivity(_sensivity), distance(_distance), offset(_offset)
		{ }

		/// Methods (public)
		void update() override
		{
#ifdef DEBUG
			Debug::drawVector(vec3(0.0f), vec3(1, 0, 0), vec3(1, 0, 0));
			Debug::drawVector(vec3(0.0f), vec3(0, 1, 0), vec3(0, 1, 0));
			Debug::drawVector(vec3(0.0f), vec3(0, 0, 1), vec3(0, 0, 1));
#endif

			angles += radians(Input::getMouseDelta() * sensivity);
			angles.y = clamp(angles.y, clampAngleY.x, clampAngleY.y);

			distance = max(0.01f, distance - 0.2f*Input::getMouseWheelDelta());
		}

		void lateUpdate() override
		{
			if (Input::getCursorMode() != CursorMode::Capture)
				return;


			if (target == nullptr) /// FPS
			{
				vec3 dir = getMovement(tr->getVectorToWorldSpace(vec3(1, 0, 0)));
				if (dir == vec3(0.0f))
					return;

				float speed = (Input::getKeyDown(sf::Keyboard::LShift)?5.0f:12.0f) * Time::deltaTime;

				tr->position += dir * speed;
				tr->rotation = quat(vec3(0.0f, angles.y, angles.x));
			}

			else /// TPS
			{
				tr->position = target->getToWorldSpace(offset) -
					quat(vec3(0.0f, angles.y, angles.x)) *
					vec3(distance, 0.0f, 0.0f);
				tr->lookAt(target->getToWorldSpace(offset));
			}


			tr->toMatrix();
		}

		void lookAt(vec3 _pos)
		{
			tr->lookAt(_pos);
			vec3 ea = eulerAngles(tr->rotation);

			angles.x = ea.z;
			angles.y = ea.y;
		}

		static vec3 getMovement(vec3 direction)
		{
			vec3 dir = vec3(0.0f);

			vec3 up = vec3(0, 0, 1);
			vec3 right = normalize(cross(direction, up));

			if (Input::getKeyDown(sf::Keyboard::Z))	 dir += direction;
			if (Input::getKeyDown(sf::Keyboard::S))	 dir -= direction;
			if (Input::getKeyDown(sf::Keyboard::D))	 dir += right;
			if (Input::getKeyDown(sf::Keyboard::Q))	 dir -= right;
			if (Input::getKeyDown(sf::Keyboard::Space))		dir += up;
			if (Input::getKeyDown(sf::Keyboard::LControl))	dir -= up;

			return dir;
		}

	private:
		Transform* target;

		vec2 angles;
		vec2 clampAngleY;

		float sensivity;
		float distance;

		vec3 offset;
};
