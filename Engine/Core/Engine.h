#pragma once

#include "Core/System.h"
#include "Math/glm.h"

#define NEW_SYSTEM(type) new(Engine::alloc_system(#type)) type

struct Engine
{
	static void init(int argc, char **argv, struct SDL_Window *window);
	static void load();
	static void clear();
	static void destroy();
	static void frame();

	// systems
	static void register_system_type(const system_type_t &type);
	static const system_type_t *system_type(const char *type_name);
	static const system_type_t *system_type(const void *instance);
	static void *alloc_system(const char *type_name);
	static void free_system(void *system);

	static void read_lock(void *system);
	static void read_unlock(void *system);
	static void write_lock(void *system);
	static void write_unlock(void *system);
};
