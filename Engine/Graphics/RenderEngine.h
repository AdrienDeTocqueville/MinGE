#pragma once

struct RenderEngine
{
private:
	static void init();
	static void destroy();

	friend struct Engine;
};
