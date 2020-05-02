#pragma once

#include <MinGE.h>
#include "Transform/Transform.h"

struct CameraControl
{
public:
	CameraControl(TransformSystem *_world, float _sensivity = 0.2f, float _distance = 5.0f, vec3 _offset = vec3(0.0f)):
		world(_world), angles(0.0f), clampAngleY(-0.499f*PI, 0.499f*PI),
		sensivity(_sensivity), distance(_distance), offset(_offset)
	{
		Input::set_cursor_mode(Input::Free);
	}

	void add(Entity camera)
	{
		cam = camera;
		set_target(Entity::none);
	}

	void set_target(Entity e)
	{
		target = e;
		if (target == Entity::none)
		{
			vec3 ea = world->get(cam).euler_angles();
			angles = vec2(ea.z, ea.y);
		}
	}

	/// Methods (public)
	void update()
	{
		if (Input::button_pressed(sf::Mouse::Left))
		{
			mp_saved = Input::mouse_position();
			Input::set_cursor_mode(Input::Capture);
		}
		else if (Input::button_released(sf::Mouse::Left))
		{
			Input::set_cursor_mode(Input::Free);
			Input::set_mouse_position(mp_saved);
		}

		if (Input::cursor_mode() == Input::Capture)
		{
			angles += radians(Input::mouse_delta() * sensivity);
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

		Transform tr = world->get(cam);

		if (target == Entity::none) /// FPS
		{
			vec3 dir = getMovement(tr.vec_to_world(vec3(1, 0, 0)));
			if (dir == vec3(0.0f) && Input::mouse_delta() == vec2(0.0f))
				return;

			float speed = (Input::key_down(sf::Keyboard::LShift)?0.5f:1.0f) * distance * Time::delta_time;

			tr.translate(dir * speed);
			tr.set_rotation(quat(vec3(0.0f, angles.y, angles.x)));
		}

		else /// TPS
		{
			Transform obj = world->get(target);
			tr.set_position(obj.to_world(offset) -
				quat(vec3(0.0f, angles.y, angles.x)) *
				vec3(distance, 0.0f, 0.0f));
			tr.look_at(obj.to_world(offset));
		}
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

	Entity cam;
	Entity target;

	vec2 angles;
	vec2 clampAngleY;
	vec2 mp_saved;

	float sensivity;
	float distance;

	vec3 offset;

	static const system_type_t type;
};

const system_type_t CameraControl::type = []() {
	system_type_t t{};
	t.name = "CameraControl";
	t.size = sizeof(CameraControl);
	t.on_main_thread = 0;

	t.destroy = [](void *system) { ((CameraControl*)system)->~CameraControl(); };
	t.update = [](void *system) { ((CameraControl*)system)->update(); };
	t.serialize = NULL;
	t.deserialize = NULL;
	return t;
}();
