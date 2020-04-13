#pragma once

#include "Math/glm.h"

struct AABB
{
	static bool collide(const AABB &a, const AABB &b);
	bool operator==(const AABB& box);

	void init(vec3 _min, vec3 _max);
	void extend(const AABB& box);

	vec3	center()	const { return 0.5f * (bounds[0] + bounds[1]); }
	vec3	dim()		const { return bounds[1] - bounds[0]; }
	float	volume()	const;

	vec3 bounds[2];
};
