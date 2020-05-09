#pragma once

#include "Core/System.h"
#include "Math/glm.h"

namespace sf
{ class RenderWindow; }

struct Engine
{
	static void init(sf::RenderWindow &window, unsigned _FPS = 60);
	static void destroy();

	static void register_system_type(const system_type_t &system_type);
	static void *alloc_system(const char *type_name);

	static void read_lock(void *system);
	static void read_unlock(void *system);
	static void write_lock(void *system);
	static void write_unlock(void *system);

	static const system_type_t *get_system_type(void *system);

	static void update();
};
