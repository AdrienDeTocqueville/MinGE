#include "Transform/Transform.h"
#include "IO/json.hpp"

using namespace nlohmann;

Transform TransformSystem::add(Entity entity, vec3 position, quat rotation, vec3 scale)
{
	assert(entity.id() != 0 && "Invalid entity");
	assert(indices.find(entity.id()) == indices.end() && "Entity already has a Transform component");

	uint32_t i = data.add();
	indices[entity.id()] = i;

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
	assert(parent.id() != 0 && "Invalid parent");
	assert(has(parent) && "Parent has no Transform component");

	assert(entity.id() != 0 && "Invalid entity");
	assert(indices.find(entity.id()) == indices.end() && "Entity already has a Transform component");

	uint32_t p = indices[parent.id()];
	uint32_t i = data.add();
	indices[entity.id()] = i;

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
	auto it = indices.find(entity.id());
	const uint32_t i = it->second;
	indices.erase(it);

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


/// TYPE DEFINITION
static inline json to_json(vec3 v)
{
	return {{"x", v.x}, {"y", v.y}, {"z", v.z}};
}

static json serialize(void *system)
{
	json dump;

	auto sys = (TransformSystem*)system;
	TransformSystem::transform_t *transforms = sys->data.get<0>();
	TransformSystem::hierarchy_t *hierarchies = sys->data.get<1>();

	dump["indices"] = json::object();
	dump["transforms"] = json::array();
	dump["hierarchies"] = json::array();

	std::map<uint32_t, uint32_t> components;
	for (auto &pair : sys->indices)
		components[pair.second] = pair.first;

	for (auto &pair : components)
	{
		dump["indices"][std::to_string(pair.second)] = pair.first;

		json transform = json::object();
		transform["position"] = ::to_json(transforms[pair.first].position);
		dump["transforms"].push_back(transform);

		json hierarchy = json::object();
		hierarchy["parent"] = hierarchies[pair.first].parent;
		dump["hierarchies"].push_back(hierarchy);
	}
	return dump;
}

TransformSystem::TransformSystem(const json &dump)
{ /*TODO*/ }

const system_type_t TransformSystem::type = []() {
	system_type_t t{};
	t.name = "TransformSystem";
	t.size = sizeof(TransformSystem);

	t.destroy = [](void *system) { ((TransformSystem*)system)->~TransformSystem(); };
	t.update = NULL;
	t.serialize = serialize;
	t.deserialize = [](void *system, const json &dump) { new(system) TransformSystem(dump); };
	return t;
}();
