#pragma once

#include "Core/System.h"
#include "Math/glm.h"

namespace sf
{ class RenderWindow; }

class Engine
{
public:
	static void init(sf::RenderWindow &window, unsigned _FPS = 60);
	static void destroy();

	static void register_system_type(const system_type_t &system_type);
	static void *alloc_system(const char *type_name, const void **dependencies = NULL, uint32_t dependency_count = 0);

	static const system_type_t *get_system_type(void *system);

	static void start_frame();
	static void end_frame();
};
