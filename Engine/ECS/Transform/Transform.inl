#pragma once

// TransformSystem
Transform TransformSystem::add(Entity entity, vec3 position, quat rotation, vec3 scale)
{
	assert(entity.id() != 0, "Invalid entity");
	assert(indices.find(entity.id()) == indices.end(), "Entity already has a Transform component");

	uint32_t i = data.add();
	indices[entity.id()] = i;

	// Init transform
	data.transforms[i].position = position;
	data.transforms[i].rotation = rotation;
	data.transforms[i].scale    = scale;

	simd_mul(data.transforms[i].world, glm::translate(position), glm::toMat4(rotation));
	simd_mul(data.transforms[i].world, data.transforms[i].world, glm::scale(scale));
	data.transforms[i].local  = glm::inverse(data.transforms[i].world);

	// Init hierarchy
	data.hierarchies[i].first_child = 0;
	data.hierarchies[i].parent = 0;
	data.hierarchies[i].next_sibling = 0;

	return Transform(i, *this);
}

Transform TransformSystem::add_child(Entity parent, Entity entity, vec3 position, quat rotation, vec3 scale)
{
	assert(parent.id() != 0, "Invalid parent");
	assert(has(parent), "Parent has no Transform component");

	assert(entity.id() != 0, "Invalid entity");
	assert(indices.find(entity.id()) == indices.end(), "Entity already has a Transform component");

	uint32_t p = indices[parent.id()];
	uint32_t i = data.add();
	indices[entity.id()] = i;

	// Init transform
	data.transforms[i].position = position;
	data.transforms[i].rotation = rotation;
	data.transforms[i].scale    = scale;

	simd_mul(data.transforms[i].world, data.transforms[p].world, glm::translate(position));
	simd_mul(data.transforms[i].world, data.transforms[i].world, glm::toMat4(rotation));
	simd_mul(data.transforms[i].world, data.transforms[i].world, glm::scale(scale));
	data.transforms[i].local  = glm::inverse(data.transforms[i].world);

	// Init hierarchy
	data.hierarchies[i].first_child = 0;
	data.hierarchies[i].parent = p;
	data.hierarchies[i].next_sibling = data.hierarchies[p].first_child;
	data.hierarchies[p].first_child = i;

	return Transform(i, *this);
}

void TransformSystem::remove(Entity entity)
{
	auto it = indices.find(entity.id());
	const uint32_t i = it->second;
	indices.erase(it);

	// Patch parent hierarchy
	if (const uint32_t p = data.hierarchies[i].parent)
	{
		if (data.hierarchies[p].first_child == i)
			data.hierarchies[p].first_child = data.hierarchies[i].next_sibling;
		else
		{
			uint32_t child = data.hierarchies[p].first_child;
			while (data.hierarchies[child].next_sibling != i)
				child = data.hierarchies[child].next_sibling;
			data.hierarchies[child].next_sibling = data.hierarchies[i].next_sibling;
		}
	}
	// Patch children
	if (uint32_t child = data.hierarchies[i].first_child)
	{
		while (data.hierarchies[child].next_sibling != 0)
		{
			data.hierarchies[child].parent = 0;
			child = data.hierarchies[child].next_sibling;
		}
	}

	data.remove(i);
}

void TransformSystem::update_matrices(uint32_t i)
{
	const uint32_t parent = data.hierarchies[i].parent;
	uint32_t child = data.hierarchies[i].first_child;

	if (parent)
	{
		simd_mul(data.transforms[i].world, data.transforms[parent].world, glm::translate(data.transforms[i].position));
		simd_mul(data.transforms[i].world, data.transforms[i].world, glm::toMat4(data.transforms[i].rotation));
		simd_mul(data.transforms[i].world, data.transforms[i].world, glm::scale(data.transforms[i].scale));
	}
	else
	{
		simd_mul(data.transforms[i].world, glm::translate(data.transforms[i].position), glm::toMat4(data.transforms[i].rotation));
		simd_mul(data.transforms[i].world, data.transforms[i].world, glm::scale(data.transforms[i].scale));
	}
	data.transforms[i].local = glm::inverse(data.transforms[i].world);


	while (child)
	{
		update_matrices(child);
		child = data.hierarchies[child].next_sibling;
	}
}

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
TransformSystem::multi_array_t::multi_array_t():
	capacity(0x1000), last(0), next_slot(0)
{
	transforms  = (transform_t*)calloc(capacity, size_sum) - 1;
	hierarchies = (hierarchy_t*)(transforms + 1 + capacity) - 1;
}

TransformSystem::multi_array_t::~multi_array_t()
{
	free(transforms + 1);
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
	last -= (index == last);
}
