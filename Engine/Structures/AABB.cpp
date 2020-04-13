#include "Structures/AABB.h"

bool AABB::collide(const AABB &a, const AABB &b)
{
	for (unsigned i(0) ; i < 3 ; i++)
	{
		if (a.bounds[1][i] <= b.bounds[0][i] || a.bounds[0][i] >= b.bounds[1][i])
			return false;
	}

	return true;
}

void AABB::init(vec3 _min, vec3 _max)
{
	bounds[0] = _min;
	bounds[1] = _max;
}

void AABB::extend(const AABB& box)
{
	bounds[0] = min(bounds[0], box.bounds[0]);
	bounds[1] = max(bounds[1], box.bounds[1]);
}

float AABB::volume() const
{
	 vec3 d = bounds[1] - bounds[0];
	 return d.x * d.y * d.z;
}

bool AABB::operator==(const AABB& box)
{
    return (bounds[0] == box.bounds[0] &&
			bounds[1] == box.bounds[1]);
}
