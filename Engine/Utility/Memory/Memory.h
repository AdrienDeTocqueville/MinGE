#pragma once

static inline uint32_t align(uint32_t val, uint32_t alignment)
{
	return ((val + alignment - 1) / alignment ) * alignment;
}
