#pragma once

#include "Engine.h"
#include <cstdint>

struct Entity
{
	Entity(): index(0) {}
	inline uint32_t id() { return index; }

	static inline Entity create() { return Entity(++Engine::next_entity); }

private:
	Entity(uint32_t _index): index(_index) {}
	uint32_t index;
};
