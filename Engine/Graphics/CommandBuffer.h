#pragma once

#include <cstdint>

struct cmd_buffer_t
{
	uint8_t *buffer;
	uint32_t size, capacity;
};
