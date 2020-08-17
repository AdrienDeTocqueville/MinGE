#pragma once

#include "Math/glm.h"

struct Debug
{
	static void point(vec3 _point, vec3 color = vec3(1.0f, 0.1f, 0.1f));

	static void line(vec3 _from, vec3 _to, vec3 color = vec3(1.0f));
	static void vector(vec3 _point, vec3 _vector, vec3 color = vec3(1.0f));

	static void aabb(const struct AABB &box, vec3 color = vec3(1.0f));
	static void obb(const struct OBB &box, vec3 color = vec3(1.0f));
	static void sphere(const struct Sphere &sphere, vec3 color = vec3(1.0f));
	static void frustum(const struct Frustum &f, vec3 color = vec3(1.0f));

	static struct cmd_buffer_t &cmd();
	static void flush();

private:
	static void init();
	static void load();
	static void destroy();

	friend struct RenderEngine;
};
