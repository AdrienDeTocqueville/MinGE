#pragma once

#include "Core/Entity.h"
#include "Core/System.h"

#include "Math/glm.h"
#include "Structures/MultiArray.h"
#include "Structures/EntityMapper.h"

struct Transform: public UID32
{
	inline void set_position(vec3 pos);
	inline void set_rotation(vec3 rot);
	inline void set_rotation(quat rot);
	inline void set_scale(vec3 scale);

	inline void translate(vec3 vec);
	inline void rotate(vec3 axis, float angle);
	inline void look_at(vec3 point);

	inline vec3 position() const;
	inline vec3 euler_angles() const;
	inline quat rotation() const;
	inline vec3 scale() const;

	inline const mat4 &world_matrix() const;
	inline const mat4 &local_matrix() const;

	inline vec3 to_world(vec3 point) const;
	inline vec3 vec_to_world(vec3 vec) const;

private:
	friend struct TransformSystem;

	Transform(uint32_t id, TransformSystem &system): UID32(id, 0), sys(system) {}
	TransformSystem &sys;
};


struct TransformSystem
{
	TransformSystem() = default;
	TransformSystem(const nlohmann::json &dump);

	Transform add(Entity entity, vec3 position, quat rotation, vec3 scale);
	Transform add_child(Entity parent, Entity entity, vec3 position, quat rotation, vec3 scale);
	void remove(Entity entity);

	Transform add(Entity entity, vec3 position = vec3(0.0f), vec3 rotation = vec3(0.0f), vec3 scale = vec3(1.0f))
	{ return add(entity, position, quat(rotation), scale); }

	Transform add_child(Entity parent, Entity entity, vec3 position = vec3(0.0f), vec3 rotation = vec3(0.0f), vec3 scale = vec3(1.0f))
	{ return add_child(parent, entity, position, quat(rotation), scale); }

	bool has(Entity entity)	const	{ return indices.has(entity); }
	Transform get(Entity entity)	{ return Transform(indices.get(entity), *this); }


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

	entity_mapper_t<1> indices;
	multi_array_t<transform_t, hierarchy_t> data;

	static const system_type_t type;

	void update_matrices(uint32_t i);
	void clear();
};


#include "Transform/Transform.inl"
