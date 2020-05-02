#pragma once

#include "Math/glm.h"

namespace RenderEngine
{ void init(); }

class Debug
{
	friend struct GraphicsSystem;
	friend void RenderEngine::init();

public:
	static void point(vec3 _point, vec3 _color = vec3(1.0f, 0.1f, 0.1f));

	static void line(vec3 _from, vec3 _to, vec3 _color = vec3(1.0f));
	static void vector(vec3 _point, vec3 _vector, vec3 _color = vec3(1.0f));

	static void aabb(const struct AABB &box, vec3 color = vec3(1.0f));
	static void frustum(const struct Frustum &f, vec3 color = vec3(1.0f));

	static void flush();

private:
	static void init();
	static void destroy();
};
