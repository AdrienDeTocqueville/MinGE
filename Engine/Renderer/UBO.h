#pragma once

#include "Renderer/GLDriver.h"

struct UBO
{
	GLuint res;
	uint32_t offset, size;
	uint8_t *data;

	static void setupPool();
	static UBO create(uint32_t size);
};
