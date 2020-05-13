#pragma once

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "UI/nuklear.h"

struct UI
{
	static void send_inputs();
	static void flush();

private:
	static void init();
	static void destroy();

	static struct nk_context *ctx;

	friend struct RenderEngine;
};
