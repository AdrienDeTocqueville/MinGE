#pragma once

#include "Core/UID.h"

struct Entity: public UID32
{
	Entity() {}

	static const Entity none;
	static inline Entity create() { return Entity(next_index++); }

private:
	Entity(uint32_t _index): UID32(_index, 0) {}

	static uint32_t next_index;
	friend class Scene; // for serialization
};
