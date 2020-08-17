#pragma once

#include "Core/System.h"
#include "Math/glm.h"

struct Engine
{
	static void init(struct SDL_Window *window);
	static void load();
	static void clear();
	static void destroy();
	static void frame();

	// systems
	static void register_system_type(const system_type_t &system_type);
	static const system_type_t *system_type(void *system);
	static void *alloc_system(const char *type_name);
	static void free_system(void *system);

	static void read_lock(void *system);
	static void read_unlock(void *system);
	static void write_lock(void *system);
	static void write_unlock(void *system);

	// assets
	static void register_asset_type(const struct asset_type_t &asset_type);
};
