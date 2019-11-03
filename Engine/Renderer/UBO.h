#pragma once

#include "Renderer/GLDriver.h"

struct UBO
{
	uint32_t binding, res;
	uint32_t offset, size;
	uint8_t *data;

	inline void bind() const
	{
		GL::BindBufferRange(binding, res, offset, size);
	}


	static void setupPool();
	static UBO create(uint32_t binding, uint32_t size);
	static void release(UBO& ubo);
};
