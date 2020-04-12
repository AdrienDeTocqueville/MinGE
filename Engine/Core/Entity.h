#pragma once

#include <cstdint>

struct Entity
{
	Entity(): index(0) {}
	inline uint32_t id() { return index; }

	static const Entity none;
	static inline Entity create() { return Entity(next_index++); }

private:
	Entity(uint32_t _index): index(_index) {}
	uint32_t index;

	static uint32_t next_index;
	friend class Scene; // for serialization
};
