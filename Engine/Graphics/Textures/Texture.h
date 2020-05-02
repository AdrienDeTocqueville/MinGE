#pragma once

#include "Core/UID.h"
#include "Graphics/GLDriver.h"

struct Texture: public UID64
{
	Texture() {}

	bool is_valid();
	uvec2 size();
	void destroy();

	static const Texture none;
	static Texture import(const char *URI);
	static void clear();

private:
	Texture(uint32_t i): UID64(i, 0) {}
};
