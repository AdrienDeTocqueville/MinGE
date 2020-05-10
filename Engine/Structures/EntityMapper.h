#pragma once

#include "Core/Entity.h"

template<int N>
struct entity_mapper_t
{
	entity_mapper_t(): size(32) { indices = (uint32_t*)calloc(N, size * sizeof(uint32_t)); }
	~entity_mapper_t() { free(indices); }

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
	void map(Entity entity, uint32_t component)
	{
		assert(entity.id() != 0 && "Invalid entity");
		assert(!has<n>(entity) && "Entity already has such component");

		uint32_t *indices_realloc(uint32_t *indices, uint32_t &size, uint32_t new_size, int N);

		if (entity.id() >= size)
			indices = indices_realloc(indices, size, entity.id(), N);
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
};
