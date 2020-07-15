#pragma once

#include "Math/glm.h"

struct render_texture_t
{
	enum Format
	{
		DEPTH_COMPONENT16,
		DEPTH_COMPONENT24,
		DEPTH24_STENCIL8,
		DEPTH_COMPONENT32,
	};

	void create(ivec2 size, Format format);
	void destroy();

	unsigned handle = 0;
};
