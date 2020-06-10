#pragma once

#include "Core/Entity.h"
#include "IO/json_fwd.hpp"

template<int N>
struct entity_mapper_t
{
	entity_mapper_t(): size(32) { indices = (uint32_t*)calloc(N, size * sizeof(uint32_t)); }
	~entity_mapper_t() { free(indices); }

	template<int n = 0>
	inline uint32_t get(uint32_t id) const
	{
		assert(id < size);
		return indices[id + size * n];
	}

	template<int n = 0>
	inline bool has(Entity entity) const
	{
		auto id = entity.id();
		return id < size && indices[id + size * n] != 0;
	}

	template<int n = 0>
	uint32_t get(Entity entity) const
	{
		assert(entity.id() != 0 && "Invalid entity");
		assert(has<n>(entity) && "Entity has no such component");

		return indices[entity.id() + size * n];
	}

	template<int n = 0>
	void map(uint32_t id, uint32_t component) const
	{
		assert(id < size);
		indices[id + size * n] = component;
	}

	template<int n = 0>
	void map(Entity entity, uint32_t component)
	{
		assert(entity.id() != 0 && "Invalid entity");
		assert(!has<n>(entity) && "Entity already has such component");

		uint32_t *entity_mapper_realloc(uint32_t*, uint32_t&, uint32_t, int);

		if (entity.id() >= size)
			indices = entity_mapper_realloc(indices, size, entity.id(), N);
		indices[entity.id() + size * n] = component;
	}

	template<int n = 0>
	void unmap(Entity entity)
	{
		assert(entity.id() != 0 && "Invalid entity");
		assert(has<n>(entity) && "Entity has no such component");

		indices[entity.id() + size * n] = 0;
	}

	void clear()
	{
		memset(indices, 0, size * N * sizeof(uint32_t));
	}

	uint32_t size;
	uint32_t *indices;


	// Serialization
	entity_mapper_t(const nlohmann::json &dump);
	nlohmann::json to_json() const;
};
