#pragma once

#include <MinGE.h>
#include "Transform/Transform.h"
#include "Graphics/Graphics.h"

struct CameraControl
{
public:
	CameraControl(TransformSystem *_world, GraphicsSystem *_graphics, float _sensivity = 0.2f, float _distance = 15.0f, vec3 _offset = vec3(0.0f)):
		world(_world), graphics(_graphics), angles(0.0f), clampAngleY(-0.499f*PI, 0.499f*PI),
		sensivity(_sensivity), distance(_distance), offset(_offset)
	{ }

	std::vector<Entity> cameras;
	int curr = 0;

	void add(Entity camera)
	{
		cameras.push_back(camera);
		if (cam == Entity::none)
		{
			cam = camera;
			set_target(Entity::none);
		}
	}

	void set_target(Entity e)
	{
		target = e;
		if (e == Entity::none)
		{
			vec3 ea = world->get(cam).euler_angles();
			angles = vec2(ea.z, ea.y);
		}
	}

	/// Methods (public)
	void update()
	{
		if (Input::key_pressed(sf::Keyboard::Tab))
		{
			curr = (curr + 1) % cameras.size();
			cam = cameras[curr];
			set_target(Entity::none);
		}

		if (cam == Entity::none)
			return;

		if (Input::button_down(sf::Mouse::Middle))
		{
			angles += radians((vec2)Input::mouse_delta() * sensivity);
			angles.y = clamp(angles.y, clampAngleY.x, clampAngleY.y);
		}

		distance = max(0.01f, distance + (target.id() ? -1 : 1) * 0.2f*Input::wheel_scroll());

		Debug::vector(vec3(0.0f), vec3(1, 0, 0), vec3(1, 0, 0));
		Debug::vector(vec3(0.0f), vec3(0, 1, 0), vec3(0, 1, 0));
		Debug::vector(vec3(0.0f), vec3(0, 0, 1), vec3(0, 0, 1));

#ifdef DEBUG
/*
		if (Entity *light = Entity::findByTag("Sun", false))
		{
			const vec3 view_pos = light->find<Transform>()->getPosition();
			const vec3 view_dir = light->find<Light>()->getDirection();

			Debug::drawVector(view_pos, view_dir);
		}
*/
#endif

		Engine::read_lock(graphics);
		for (int i = 1; i < cameras.size(); i++)
		{
			uint32_t c = graphics->indices.get<0>(cameras[i]);
			Debug::frustum(graphics->cameras.get<0>()[c].frustum, cam == cameras[i] ? vec3(0,1,0) : vec3(1));
		}
		Engine::read_unlock(graphics);

		Engine::read_lock(world);
		Transform tr = world->get(cam);

		if (target == Entity::none) /// FPS
		{
			vec3 dir = getMovement(tr.vec_to_world(vec3(1, 0, 0)));
			if (dir != vec3(0.0f) || Input::mouse_delta() != ivec2(0))
			{
				float speed = (Input::key_down(sf::Keyboard::LShift) ? 0.5f : 1.0f) * distance * Time::delta_time;

				tr.translate(dir * speed);
				tr.set_rotation(quat(vec3(0.0f, angles.y, angles.x)));
			}
		}

		else /// TPS
		{
			Transform obj = world->get(target);
			tr.set_position(obj.to_world(offset) -
				quat(vec3(0.0f, angles.y, angles.x)) *
				vec3(distance, 0.0f, 0.0f));
			tr.look_at(obj.to_world(offset));
		}
		Engine::read_unlock(world);
	}

	void look_at(vec3 pos)
	{
		Transform tr = world->get(cam);
		tr.look_at(pos);
		vec3 ea = tr.euler_angles();

		angles.x = ea.z;
		angles.y = ea.y;
	}

	static vec3 getMovement(vec3 direction)
	{
		vec3 dir = vec3(0.0f);

		vec3 up = vec3(0, 0, 1);
		vec3 right = normalize(cross(direction, up));

		if (Input::key_down(sf::Keyboard::Z))	dir += direction;
		if (Input::key_down(sf::Keyboard::S))	dir -= direction;
		if (Input::key_down(sf::Keyboard::D))	dir += right;
		if (Input::key_down(sf::Keyboard::Q))	dir -= right;
		if (Input::key_down(sf::Keyboard::Space))	dir += up;
		if (Input::key_down(sf::Keyboard::LControl))	dir -= up;

		return dir;
	}

	TransformSystem *world;
	GraphicsSystem *graphics;

	Entity cam;
	Entity target;

	vec2 angles;
	vec2 clampAngleY;

	float sensivity;
	float distance;

	vec3 offset;

	static const system_type_t type;
};

const system_type_t CameraControl::type = []() {
	system_type_t t{};
	t.name = "CameraControl";
	t.size = sizeof(CameraControl);

	t.destroy = [](void *system) { ((CameraControl*)system)->~CameraControl(); };
	t.update = [](void *system) { ((CameraControl*)system)->update(); };
	t.serialize = NULL;
	t.deserialize = NULL;
	return t;
}();
