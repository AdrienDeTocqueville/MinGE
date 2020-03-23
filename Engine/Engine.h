#pragma once

#include "Utility/glm.h"

namespace sf
{ class RenderWindow; }

class Engine
{
public:
	static void init(sf::RenderWindow &window, unsigned _FPS = 60);
	static void destroy();

	static void start_frame();
	static void end_frame();

	static void setWindowSize(vec2 _newSize);
};
