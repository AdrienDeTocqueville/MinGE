#pragma once

#include "includes.h"

#define DRAWAABB

struct AABB
{
	static bool collide(AABB* a, AABB* b);
	bool operator==(const AABB& box);

	void compute(const std::vector<vec3>& points);
	void extend(const AABB& box);

	vec3	center()	const { return 0.5f * (bounds[0] + bounds[1]); }
	vec3	dim()		const { return bounds[1] - bounds[0]; }
	float	volume()	const;

	vec3 bounds[2];

#ifdef DRAWAABB
	void prepare(float padding = 0.0f);
	static void draw();

	vec3 default_color = vec3(0.3f, 0.78f, 0.84f);
	vec3 color;

	static unsigned vbo;
	static unsigned vao;

	static std::vector<vec3> vertices;
	static std::vector<vec3> colors;

	static bool drawAABBs;
#endif
};
