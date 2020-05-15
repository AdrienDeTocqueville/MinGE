#pragma once

#include "Math/glm.h"
#include "UI/imgui/imgui.h"

struct UI
{
	static void send_inputs();
	static void flush();

private:
	static void init();
	static void destroy();

	friend struct RenderEngine;
};
