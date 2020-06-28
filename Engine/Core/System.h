#pragma once

#include <stdint.h>

struct SerializationContext;
struct system_type_t
{
	const char *name;
	uint32_t size;

	void (*destroy)(void *instance);
	void (*update)(void *instance);

	void (*on_destroy_entity)(void *instance, struct Entity);
	void (*on_resize_window)(void *instance);

	void (*save)(void *instance, SerializationContext&);
	void (*load)(void *instance, const SerializationContext&);
};
