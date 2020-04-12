#pragma once

#include "ECS/System.h"
#include "Utility/glm.h"

namespace sf
{ class RenderWindow; }

class Engine
{
public:
	static void init(sf::RenderWindow &window, unsigned _FPS = 60);
	static void destroy();

	static void register_system_type(const system_type_t &system_type);
	static void *create_system(const char *type_name, const void **dependencies);

	static const system_type_t *get_system_type(void *system);

	static void start_frame();
	static void end_frame();

	static void setWindowSize(vec2 _newSize);
};