#pragma once

#include <stdint.h>

struct cmd_buffer_t;

struct RenderEngine
{
	static uint32_t create_cmd_buffer();
	static cmd_buffer_t &get_cmd_buffer(uint32_t i);
	static void destroy_cmd_buffer(uint32_t i);

private:
	static void init();
	static void destroy();

	static void flush();

	friend struct Engine;
};
