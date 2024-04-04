#include "Transform/Transform.h"

#include "IO/DataModel.h"
#include "Structures/EntityMapper.inl"

Transform TransformSystem::add(Entity entity, vec3 position, quat rotation, vec3 scale)
{
	uint32_t i = data.add();
	indices.map(entity, i);

	transform_t *transforms = data.get<0>();
	hierarchy_t *hierarchies = data.get<1>();

	// Init transform
	transforms[i].position = position;
	transforms[i].rotation = rotation;
	transforms[i].scale    = scale;

	simd_mul(transforms[i].world, glm::translate(position), glm::toMat4(rotation));
	simd_mul(transforms[i].world, transforms[i].world, glm::scale(scale));
	transforms[i].local  = glm::inverse(transforms[i].world);

	// Init hierarchy
	hierarchies[i].first_child = 0;
	hierarchies[i].parent = 0;
	hierarchies[i].next_sibling = 0;

	return Transform(i, *this);
}

Transform TransformSystem::add_child(Entity parent, Entity entity, vec3 position, quat rotation, vec3 scale)
{
	uint32_t p = indices.get(parent);
	uint32_t i = data.add();
	indices.map(entity, i);

	transform_t *transforms = data.get<0>();
	hierarchy_t *hierarchies = data.get<1>();

	// Init transform
	transforms[i].position = position;
	transforms[i].rotation = rotation;
	transforms[i].scale    = scale;

	simd_mul(transforms[i].world, transforms[p].world, glm::translate(position));
	simd_mul(transforms[i].world, transforms[i].world, glm::toMat4(rotation));
	simd_mul(transforms[i].world, transforms[i].world, glm::scale(scale));
	transforms[i].local  = glm::inverse(transforms[i].world);

	// Init hierarchy
	hierarchies[i].first_child = 0;
	hierarchies[i].parent = p;
	hierarchies[i].next_sibling = hierarchies[p].first_child;
	hierarchies[p].first_child = i;

	return Transform(i, *this);
}

void TransformSystem::remove(Entity entity)
{
	uint32_t i = indices.get(entity);
	indices.unmap(entity);

	transform_t *transforms = data.get<0>();
	hierarchy_t *hierarchies = data.get<1>();

	// Patch parent hierarchy
	if (const uint32_t p = hierarchies[i].parent)
	{
		if (hierarchies[p].first_child == i)
			hierarchies[p].first_child = hierarchies[i].next_sibling;
		else
		{
			uint32_t child = hierarchies[p].first_child;
			while (hierarchies[child].next_sibling != i)
				child = hierarchies[child].next_sibling;
			hierarchies[child].next_sibling = hierarchies[i].next_sibling;
		}
	}
	// Patch children
	if (uint32_t child = hierarchies[i].first_child)
	{
		while (hierarchies[child].next_sibling != 0)
		{
			hierarchies[child].parent = 0;
			child = hierarchies[child].next_sibling;
		}
	}

	data.remove(i);
}

void TransformSystem::update_matrices(uint32_t i)
{
	transform_t *transforms = data.get<0>();
	hierarchy_t *hierarchies = data.get<1>();

	const uint32_t parent = hierarchies[i].parent;
	uint32_t child = hierarchies[i].first_child;

	if (parent)
	{
		simd_mul(transforms[i].world, transforms[parent].world, glm::translate(transforms[i].position));
		simd_mul(transforms[i].world, transforms[i].world, glm::toMat4(transforms[i].rotation));
		simd_mul(transforms[i].world, transforms[i].world, glm::scale(transforms[i].scale));
	}
	else
	{
		simd_mul(transforms[i].world, glm::translate(transforms[i].position), glm::toMat4(transforms[i].rotation));
		simd_mul(transforms[i].world, transforms[i].world, glm::scale(transforms[i].scale));
	}
	transforms[i].local = glm::inverse(transforms[i].world);


	while (child)
	{
		update_matrices(child);
		child = hierarchies[child].next_sibling;
	}
}

void TransformSystem::clear()
{
	indices.clear();
	data.clear();
}

static void transfer(TransformSystem *sys, Entity e, DataModel *model)
{
	if (!sys->has(e))
	{
		if (model->is_writing())
			return;
		sys->add(e);
	}
	Transform tr = sys->get(e);
	TransformSystem::transform_t *data = sys->data.get<0>(tr.id());

	model->transfer("position", data->position);
	model->transfer("rotation", data->rotation);
	model->transfer("scale", data->scale);

	/*
	comp["first_child"] = hierarchies[indices[i]].first_child;
	comp["parent"] = hierarchies[indices[i]].parent;
	comp["next_sibling"] = hierarchies[indices[i]].next_sibling;
	*/
}

/// TYPE DEFINITION
const system_type_t TransformSystem::type = []() {
	system_type_t t = INIT_SYSTEM_TYPE(TransformSystem);

	t.destroy = [](void *system) { ((TransformSystem*)system)->~TransformSystem(); };
	t.transfer = (decltype(system_type_t::transfer)) transfer;
	return t;
}();
