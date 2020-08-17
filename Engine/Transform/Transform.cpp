#include "Transform/Transform.h"

#include "Core/Serialization.h"
#include "Structures/EntityMapper.inl"

using namespace nlohmann;

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


/// SERIALIZATION
static void serialize(TransformSystem *sys, SerializationContext &ctx)
{
	TransformSystem::transform_t *transforms = sys->data.get<0>();
	TransformSystem::hierarchy_t *hierarchies = sys->data.get<1>();

	json dump = sys->indices.to_json();
	const uint32_t max_component = dump["max_component"][0].get<uint32_t>();
	const uint32_t *indices = sys->indices.indices;

	json data = json::array();
	data.get_ptr<nlohmann::json::array_t*>()->reserve(max_component);
	for (uint32_t i = 1; i < sys->indices.size; i++)
	{
		if (indices[i] == 0) continue;

		json comp = json::object();
		comp["position"] = ::to_json(transforms[indices[i]].position);
		comp["rotation"] = ::to_json(transforms[indices[i]].rotation);
		comp["scale"]    = ::to_json(transforms[indices[i]].scale);

		comp["first_child"]  = hierarchies[indices[i]].first_child;
		comp["parent"]       = hierarchies[indices[i]].parent;
		comp["next_sibling"] = hierarchies[indices[i]].next_sibling;

		data.push_back(std::move(comp));
	}

	dump["components"].swap(data);
	ctx.swap_data(dump);
}

TransformSystem::TransformSystem(const SerializationContext &ctx):
	indices(ctx["indices"]), data(ctx["max_component"][0].get<uint32_t>(), false)
{
	data.init(ctx["max_component"][0].get<uint32_t>());

	uint32_t final_slot = 1;
	auto *slots = data.get<0>();
	auto comp = ctx["components"].rbegin();
	for (uint32_t i = 1; i < indices.size; i++)
	{
		uint32_t idx = indices.get<0>(indices.size - i);
		if (idx == 0) continue;

		if (idx == 1) final_slot = *(uint32_t*)(slots + idx);
		else *(uint32_t*)(slots + idx - 1) = *(uint32_t*)(slots + idx);

		TransformSystem::transform_t *tr = data.get<0>(idx);
		tr->position = ::to_vec3(comp.value()["position"]);
		tr->rotation = ::to_quat(comp.value()["rotation"]);
		tr->scale    = ::to_vec3(comp.value()["scale"]);

		TransformSystem::hierarchy_t *hi = data.get<1>(idx);
		hi->first_child  = comp.value()["first_child"];
		hi->parent       = comp.value()["parent"];
		hi->next_sibling = comp.value()["next_sibling"];

		++comp;
	}
	data.next_slot = final_slot;

	for (uint32_t i = 1; i < indices.size; i++)
	{
		uint32_t idx = indices.get<0>(indices.size - i);
		if (idx == 0) continue;

		TransformSystem::hierarchy_t *hi = data.get<1>(idx);
		if (hi->parent == 0) update_matrices(idx);
	}
}

/// TYPE DEFINITION
const system_type_t TransformSystem::type = []() {
	system_type_t t = INIT_SYSTEM_TYPE(TransformSystem);

	t.destroy = [](void *system) { ((TransformSystem*)system)->~TransformSystem(); };

	t.save = (void(*)(void*, SerializationContext&))serialize;
	t.load = [](void *system, const SerializationContext &ctx) { new(system) TransformSystem(ctx); };
	return t;
}();
