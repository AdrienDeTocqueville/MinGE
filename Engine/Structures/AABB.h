#pragma once

#include "Math/glm.h"

struct Frustum
{ };

struct AABB
{
	inline static bool collide(const AABB &a, const AABB &b);
	inline static bool collide(const Frustum &f, const AABB &b);

	inline void init(vec3 _min, vec3 _max);
	inline void extend(const AABB& box);
	inline void transform(mat4 matrix);

	vec3	center()	const { return 0.5f * (bounds[0] + bounds[1]); }
	vec3	dim()		const { return bounds[1] - bounds[0]; }
	float	volume()	const { vec3 d = dim(); return d.x * d.y * d.z; }

	bool operator==(const AABB& box)
	{ return (bounds[0] == box.bounds[0]) && (bounds[1] == box.bounds[1]); }

	vec3 bounds[2];
};

inline bool AABB::collide(const AABB &a, const AABB &b)
{
	for (unsigned i(0) ; i < 3 ; i++)
	{
		if (a.bounds[1][i] <= b.bounds[0][i] || a.bounds[0][i] >= b.bounds[1][i])
			return false;
	}

	return true;
}

inline bool AABB::collide(const Frustum &f, const AABB &b)
{ return false; }

inline void AABB::init(vec3 _min, vec3 _max)
{
	bounds[0] = _min;
	bounds[1] = _max;
}

inline void AABB::extend(const AABB& box)
{
	bounds[0] = min(bounds[0], box.bounds[0]);
	bounds[1] = max(bounds[1], box.bounds[1]);
}

inline void AABB::transform(mat4 matrix)
{
	//bounds[0] *= mat3(matrix);
	//bounds[1] *= mat3(matrix);
}
