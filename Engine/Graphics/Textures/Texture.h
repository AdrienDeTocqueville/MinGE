#pragma once

#include "Core/UID.h"
#include "Graphics/GLDriver.h"

struct Texture: public UID64
{
	Texture() {}

	void destroy();
	const char *uri();
	uvec2 size();
	bool is_valid();

	static Texture load(const char *URI);
	static Texture get(uint32_t i);
	static void clear();
	static uint32_t count();

	static const Texture none;

private:
	Texture(uint32_t i, uint32_t gen): UID64(i, gen) {}
};
