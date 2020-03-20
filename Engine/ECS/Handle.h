#pragma once

#include <cstdint>

struct Handle
{
	Handle(): index(0) {}
	Handle(uint32_t _index): index(_index) {}

	inline uint32_t id() { return index; }
	uint32_t index;
};
