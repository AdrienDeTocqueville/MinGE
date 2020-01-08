#pragma once

#include <MinGE.h>

class CameraScript : public Script
{
	public:
		CameraScript(Transform* _target, float _sensivity = 0.2f, float _distance = 0.01f, vec3 _offset = vec3(0.0f)):
			target(_target), angles(0.0f), sensivity(_sensivity), distance(_distance), offset(_offset)
		{ }

		void start() override
		{
			Input::setCursorMode(Input::Free);

			if (target == nullptr)
			{
				vec3 ea = eulerAngles(tr->rotation);
				angles = vec2(ea.z, ea.y);
			}
		}

		/// Methods (public)
		void update() override
		{
			if (Input::getMousePressed(sf::Mouse::Left))
				Input::setCursorMode(Input::Capture);
			else if (Input::getMouseReleased(sf::Mouse::Left))
				Input::setCursorMode(Input::Free);

			distance = max(0.01f, distance + (target ? -1 : 1) * 0.2f*Input::getMouseWheelDelta());

			if (Input::getKeyPressed(sf::Keyboard::F5))
			{
				auto *g = Entity::findByTag("Bob")->find<Graphic>();
				for (MaterialRef m : g->getMaterials())
					m->reload();
			}

#ifdef DEBUG
			Debug::drawVector(vec3(0.0f), vec3(1, 0, 0), vec3(1, 0, 0));
			Debug::drawVector(vec3(0.0f), vec3(0, 1, 0), vec3(0, 1, 0));
			Debug::drawVector(vec3(0.0f), vec3(0, 0, 1), vec3(0, 0, 1));

			if (Entity *light = Entity::findByTag("Sun", false))
			{
				const vec3 view_pos = light->find<Transform>()->getPosition();
				const vec3 view_dir = light->find<Light>()->getDirection();

				Debug::drawVector(view_pos, view_dir);
			}
#endif

		}

		void lateUpdate() override
		{
			vec3 euler_angles(0,0,0);
			if (Input::getCursorMode() == Input::Capture && Input::getMouseDelta() != vec2(0.0f))
			{
				float y_limit = 0.5f*PI - EPSILON;
				angles += radians(Input::getMouseDelta() * sensivity);
				angles.y = clamp(angles.y, -y_limit, y_limit);
				euler_angles = vec3(0.0f, angles.y, angles.x);
			}

			if (target == nullptr) /// FPS
			{
				vec3 dir = getMovement(tr->vectorToWorld(vec3(1, 0, 0)));
				if (dir != vec3(0.0f))
				{
					float speed = (Input::getKeyDown(sf::Keyboard::LShift) ? 0.5f : 1.0f) * distance * Time::deltaTime;
					tr->position += dir * speed;
				}

				if (euler_angles != vec3(0.0f))
					tr->rotation = quat(euler_angles);
			}

			else /// TPS
			{
				tr->position = target->toWorld(offset) - quat(euler_angles) * vec3(distance, 0.0f, 0.0f);
				tr->lookAt(target->toWorld(offset));
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

		float sensivity;
		float distance;

		vec3 offset;
};
