#pragma once

#include "Core/UID.h"
#include "Structures/MultiArray.h"

struct entity_t
{
	uint32_t reserved;
	uint8_t gen;
	uint8_t destroyed;
	char *name;
};

struct Entity: public UID32
{
	Entity() {}

	void destroy();
	inline const char *name() const { return entities.get<0>(id())->name; }
	inline bool is_valid() const
	{
		auto i = id();
		return i && (i <= entities.size) && (gen() == entities.get<0>(i)->gen);
	}

	static Entity create(const char *name = NULL);
	static Entity get(uint32_t i);
	static Entity get(const char *name);
	static void clear();

	static const Entity none;
	static multi_array_t<entity_t> entities;

private:
	Entity(uint32_t _index, uint32_t _gen): UID32(_index, _gen) {}
};
