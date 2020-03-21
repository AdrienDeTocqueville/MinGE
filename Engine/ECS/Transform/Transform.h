#pragma once

#include "ECS/Entity.h"
#include "Utility/glm.h"

#include <cstdint>
#include <unordered_map>

struct Transform
{
	inline uint32_t id() const { return index; }

	void set_position(vec3 pos);
	void set_rotation(vec3 rot);
	void set_rotation(quat rot);
	void set_scale(vec3 scale);

	vec3 position() const;
	quat rotation() const;
	vec3 scale() const;

	const mat4 &world_matrix();
	const mat4 &local_matrix();

private:
	friend struct TransformSystem;

	Transform(uint32_t id, TransformSystem &system): index(id), sys(system) {}
	uint32_t index;
	TransformSystem &sys;
};


struct TransformSystem
{
	Transform add(Entity entity, vec3 position, quat rotation, vec3 scale);
	void remove(Entity entity);
	void add_child(Entity parent, Entity child);

	Transform add(Entity entity, vec3 position = vec3(0.0f), vec3 rotation = vec3(0.0f), vec3 scale = vec3(1.0f))
	{ return add(entity, position, quat(rotation), scale); }

	bool has(Entity entity)
	{ return (indices.find(entity.id()) != indices.end()); }

	Transform get(Entity entity)
	{
		assert(has(entity), "Entity has no Transform component");
		return Transform(indices[entity.id()], *this);
	}


	struct transform_t
	{
		vec3 position;
		quat rotation;
		vec3 scale;
		mat4 world, local;
	};
	struct hierarchy_t
	{
		uint32_t first_child;
		uint32_t parent, next_sibling;
	};
	struct multi_array_t
	{
		multi_array_t();
		~multi_array_t();
		void reserve(uint32_t new_cap);
		uint32_t add();
		void remove(uint32_t index);

		uint32_t next_slot, capacity, last;
		transform_t *transforms;
		hierarchy_t *hierarchies;

		static constexpr long size_sum = sizeof(transform_t) + sizeof(hierarchy_t);
	};

	std::unordered_map<uint32_t, uint32_t> indices;
	multi_array_t data;

	void update_matrices(uint32_t i)
	{
		uint32_t child = data.hierarchies[i].first_child;
		uint32_t parent = data.hierarchies[i].parent;

		data.transforms[i].world = parent
			? data.transforms[parent].world * glm::translate(data.transforms[i].position)
			: data.transforms[i].world = glm::translate(data.transforms[i].position);

		data.transforms[i].world *= glm::toMat4(data.transforms[i].rotation);
		data.transforms[i].world *= glm::scale(data.transforms[i].scale);
		data.transforms[i].local  = glm::inverse(data.transforms[i].world);

		while (child)
		{
			update_matrices(child);
			child = data.hierarchies[child].next_sibling;
		}
	}
};


#include "ECS/Transform/Transform.inl"
