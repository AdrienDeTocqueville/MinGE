#pragma once

#include <Core/Entity.h>

// Throw compile error if name does not exist
#define INIT_SYSTEM_EDITOR(name) { (sizeof(name), #name) }

struct system_editor_t
{
	const char *type_name;

	bool (*edit_system)(void *instance);
	bool (*add_system)(void **instance);
	void (*add_component)(void *instance, const char *name, Entity e);
	void (*edit_entity)(void *instance, Entity e);
};

bool entity_dropdown(const char *label, Entity *selected);

bool choose_file(const char *name, char *output, size_t size, bool save);
