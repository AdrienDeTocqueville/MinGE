#pragma once

// Transform
void Transform::set_position(vec3 pos)
{
	sys.data.transforms[index].position = pos;
	sys.update_matrices(index);
}

void Transform::set_rotation(vec3 rot)
{
	sys.data.transforms[index].rotation = quat(rot);
	sys.update_matrices(index);
}

void Transform::set_rotation(quat rot)
{
	sys.data.transforms[index].rotation = rot;
	sys.update_matrices(index);
}

void Transform::set_scale(vec3 scale)
{
	sys.data.transforms[index].scale = scale;
	sys.update_matrices(index);
}


vec3 Transform::position() const
{ return sys.data.transforms[index].position; }

quat Transform::rotation() const
{ return sys.data.transforms[index].rotation; }

vec3 Transform::scale() const
{ return sys.data.transforms[index].scale; }


const mat4& Transform::world_matrix()
{ return sys.data.transforms[index].world; }

const mat4& Transform::local_matrix()
{ return sys.data.transforms[index].local; }


// multi_array_t
#include "Memory/Memory.h"

TransformSystem::multi_array_t::multi_array_t():
	capacity(mem::page_size), last(0), next_slot(0)
{
	transforms  = (transform_t*)calloc(capacity, size_sum) - 1;
	hierarchies = (hierarchy_t*)(transforms + 1 + capacity) - 1;
}

void TransformSystem::multi_array_t::reserve(uint64_t new_cap)
{
	void *old_alloc = transforms + 1;

	void *temp = malloc(new_cap * size_sum);
	memcpy(temp, transforms + 1, last * sizeof(transform_t));
	transforms = (transform_t*)temp - 1;

	temp = (transforms + 1 + new_cap);
	memcpy(temp, hierarchies + 1, last * sizeof(hierarchy_t));
	hierarchies = (hierarchy_t*)temp - 1;

	capacity = (uint32_t)new_cap;
	free(old_alloc);
}

uint32_t TransformSystem::multi_array_t::add()
{
	if (const uint32_t ret = next_slot)
	{
		next_slot = *(uint32_t*)(transforms + next_slot);
		return ret;
	}
	if (last + 1 == capacity)
		reserve((uint64_t)capacity * 2);
	return ++last;
}

void TransformSystem::multi_array_t::remove(uint32_t index)
{
	*(uint32_t*)(transforms + index) = next_slot;
	next_slot = index;
	//last -= (index == last);
}
