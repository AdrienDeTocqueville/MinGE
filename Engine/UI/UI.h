#pragma once

#include "Math/glm.h"
#include "UI/imgui/imgui.h"

struct UI
{
	static void create_tab(const char *name, void (*callback)(void*), const void *data, size_t size);
	template <typename T>
	static void create_tab(const char *name, void (*callback)(T*), const T &data)
	{ create_tab(name, (void (*)(void*))callback, &data, sizeof(T)); }

private:
	static void init();
	static void destroy();

	static void frame();
	static void flush();

	friend struct RenderEngine;
};
