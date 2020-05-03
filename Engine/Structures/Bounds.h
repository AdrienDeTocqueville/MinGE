#pragma once

#include "Math/glm.h"

struct AABB
{
	inline void init(vec3 _min, vec3 _max);
	inline void extend(const AABB &box);
	inline void transform(const mat4 &matrix);

	vec3	center()	const { return 0.5f * (bounds[0] + bounds[1]); }
	vec3	dim()		const { return bounds[1] - bounds[0]; }
	float	volume()	const { vec3 d = dim(); return d.x * d.y * d.z; }

	bool operator==(const AABB &box)
	{ return (bounds[0] == box.bounds[0]) && (bounds[1] == box.bounds[1]); }

	vec3 bounds[2];
};

struct OBB
{
	inline void init(const AABB &box);
	inline void transform(const mat4 &matrix);

	vec3	dim()		const { return axis[0] + axis[1] + axis[2]; }

	vec3 base, axis[3];
};

struct Sphere
{
	inline void init(const AABB &box);
	inline void init(const OBB &box);
	inline void transform(const mat4 &matrix);

	vec3 center;
	float radius2;
};

struct Frustum
{
	enum Face {Near, Left, Right, Top, Bottom, Far};

	inline void init(const mat4 &matrix);
	inline vec4 normalize_plane(const vec4 &p);

	// Pointing to the inside of the frustum
	vec4 planes[6];
};


namespace Bounds
{

inline bool collide(const AABB &a, const AABB &b)
{
	for (unsigned i(0) ; i < 3 ; i++)
	{
		if (a.bounds[1][i] <= b.bounds[0][i] || a.bounds[0][i] >= b.bounds[1][i])
			return false;
	}

	return true;
}

inline bool collide(const Frustum &f, const Sphere &s)
{
	vec3 c = s.center;
	for (unsigned i(0) ; i < 6 ; i++)
	{
		vec4 p = f.planes[i];
		float d = p.x * c.x + p.y * c.y + p.z * c.z + p.w;
		if (d < 0 && d*d > s.radius2)
			return false;
	}
	return true;
}

}



/// AABB
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

inline void AABB::transform(const mat4 &matrix)
{
	vec3 a = bounds[0], b = bounds[1];
	vec3 points[8] = {
		matrix * vec4(a, 1.0f),
		matrix * vec4(b.x,a.y,a.z, 1.0f),
		matrix * vec4(b.x,b.y,a.z, 1.0f),
		matrix * vec4(a.x,b.y,a.z, 1.0f),

		matrix * vec4(b, 1.0f),
		matrix * vec4(a.x,b.y,b.z, 1.0f),
		matrix * vec4(a.x,a.y,b.z, 1.0f),
		matrix * vec4(b.x,a.y,b.z, 1.0f),
	};

	a = b = points[0];
	for (int p = 1; p < 8; p++)
	{
		a = min(a, points[p]);
		b = max(b, points[p]);
	}
	bounds[0] = a;
	bounds[1] = b;
}


/// OOBB
inline void OBB::init(const AABB &box)
{
	base = box.bounds[0];
	vec3 dim = box.dim();
	axis[0] = vec3(dim.x, 0, 0);
	axis[1] = vec3(0, dim.y, 0);
	axis[2] = vec3(0, 0, dim.z);
}

inline void OBB::transform(const mat4 &matrix)
{
	base = vec3(matrix * vec4(base, 1.0f));
	mat3 rot_scale = mat3(matrix);
	axis[0] = rot_scale * axis[0];
	axis[1] = rot_scale * axis[1];
	axis[2] = rot_scale * axis[2];
}


/// Sphere
inline void Sphere::init(const AABB &box)
{
	vec3 half_dim = 0.5f * box.dim();
	center = box.bounds[0] + half_dim;
	radius2 = length2(half_dim);
}

inline void Sphere::init(const OBB &box)
{
	vec3 half_dim = 0.5f * box.dim();
	center = box.base + half_dim;
	radius2 = length2(half_dim);
}

inline void Sphere::transform(const mat4 &m)
{
	center = vec3(m * vec4(center, 1.0f));
	vec3 scale(length2(vec3(m[0])), length2(vec3(m[1])), length2(vec3(m[2])));
	radius2 *= max(scale.x, max(scale.y, scale.z));
}


/// FRUSTUM
inline vec4 Frustum::normalize_plane(const vec4 &p)
{
	return p / length(vec3(p));
}

inline void Frustum::init(const mat4 &matrix)
{
	vec4 rowX = glm::row(matrix, 0);
	vec4 rowY = glm::row(matrix, 1);
	vec4 rowZ = glm::row(matrix, 2);
	vec4 rowW = glm::row(matrix, 3);

	planes[Near]	= normalize_plane(rowW + rowZ);
	planes[Left]	= normalize_plane(rowW + rowX);
	planes[Right]	= normalize_plane(rowW - rowX);
	planes[Top]	= normalize_plane(rowW - rowY);
	planes[Bottom]	= normalize_plane(rowW + rowY);
	planes[Far]	= normalize_plane(rowW - rowZ);
}
