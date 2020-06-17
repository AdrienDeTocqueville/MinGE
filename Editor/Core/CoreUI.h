#pragma once

#include <Core/Entity.h>

struct system_editor_t
{
	const char *type_name;

	bool (*add_system)(void **instance);
	void (*add_component)(void *instance, const char *name, Entity e);
	void (*edit_entity)(void *instance, Entity e);
};

bool entity_dropdown(const char *label, Entity *selected);
