#pragma once

#include <stdint.h>
#include "Render/Shader/Material.h"

struct cmd_buffer_t;

struct RenderEngine
{
	static void add_buffer(cmd_buffer_t *buffer);
	static void remove_buffer(cmd_buffer_t *buffer);

	static Material default_material;

private:
	static void init();
	static void destroy();

	static void start_frame();
	static void flush();

	friend struct Engine;
};
