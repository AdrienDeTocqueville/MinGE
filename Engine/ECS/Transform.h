#pragma once

#include "Utility/glm.h"
#include "ECS/Entity.h"

#include <cstdint>

#include <unordered_map>
#include <vector>

struct Transform
{
	inline uint32_t id() { return index; }

	void set_position(vec3 pos);
	void set_rotation(vec3 rot);
	void set_rotation(quat rot);
	void set_scale(vec3 scale);

	vec3 position();
	quat rotation();
	vec3 scale();

	const mat4 &world_matrix();
	const mat4 &local_matrix();

private:
	friend struct TransformSystem;

	Transform(uint32_t _index, TransformSystem *_sys): index(_index), system(_sys) {}
	uint32_t index;
	TransformSystem *system;
};

struct TransformSystem
{
	Transform add(Entity entity, vec3 position, quat rotation, vec3 scale)
	{
		assert(entity.id() != 0, "Invalid entity");
		assert(indices.find(entity.id()) == indices.end(), "Entity already has a Transform component");

		uint32_t i = data.size();
		indices[entity.id()] = i;
		data.push_back({position, rotation, scale});

		// Compute matrices
		data[i].world  = glm::translate(position);
		data[i].world *= glm::toMat4(rotation);
		data[i].world *= glm::scale(scale);
		data[i].local  = glm::inverse(data[i].world);

		return Transform(i, this);
	}

	Transform add(Entity entity, vec3 position = vec3(0.0f), vec3 rotation = vec3(0.0f), vec3 scale = vec3(1.0f))
	{ return add(entity, position, quat(rotation), scale); }

	bool has(Entity entity)
	{ return (entity.id() != 0 && indices.find(entity.id()) != indices.end()); }

	Transform get(Entity entity)
	{
		assert(has(entity), "Entity doesn't has a Transform component");
		return Transform(indices[entity.id()], this);
	}

	void update_matrices(uint32_t i)
	{
		data[i].world  = glm::translate(data[i].position);
		data[i].world *= glm::toMat4(data[i].rotation);
		data[i].world *= glm::scale(data[i].scale);
		data[i].local  = glm::inverse(data[i].world);
	}


	struct transform_t
	{
		vec3 position;
		quat rotation;
		vec3 scale;
		mat4 world, local;
	};

	std::unordered_map<uint32_t, uint32_t> indices;
	std::vector<transform_t> data;
};

#include "ECS/Transform.inl"
