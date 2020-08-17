#pragma once

#include "Math/glm.h"
#include "UI/imgui/imgui.h"

struct UI
{
	static void set_menubar(void (*callback)());
	static void create_window(void (*callback)(uint32_t), uint32_t id);

private:
	static void init();
	static void clear();
	static void destroy();

	static void frame();
	static void flush();

	friend struct RenderEngine;
};
