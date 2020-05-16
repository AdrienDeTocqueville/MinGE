#pragma once

#include "Math/glm.h"
#include "UI/imgui/imgui.h"

struct UI
{
	static void create_tab(const char *name, void (*callback)());

private:
	static void init();
	static void destroy();

	static void frame();
	static void flush();

	friend struct RenderEngine;
};
