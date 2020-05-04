#pragma once

struct cmd_buffer_t;

struct RenderEngine
{
	static cmd_buffer_t *alloc_cmd_buffer();

private:
	static void init();
	static void destroy();

	friend struct Engine;
};
