#include "ECS/Transform/Transform.h"
#include "Utility/IO/json.hpp"

using namespace nlohmann;

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

inline json to_json(vec3 v)
{
	return {{"x", v.x}, {"y", v.y}, {"z", v.z}};
}

json TransformSystem::serialize(void *system)
{
	json dump;

	auto sys = (TransformSystem*)system;
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
		transform["position"] = ::to_json(sys->data.transforms[pair.first].position);
		dump["transforms"].push_back(transform);

		json hierarchy = json::object();
		hierarchy["parent"] = sys->data.hierarchies[pair.first].parent;
		dump["hierarchies"].push_back(hierarchy);
	}
	return dump;
}

void TransformSystem::deserialize()
{ }
