#pragma once

#include <stdint.h>

#include "IO/json_fwd.hpp"

struct SerializationContext;
struct system_type_t
{
	const char *name;
	uint32_t size;

	void (*destroy)(void *instance);
	void (*update)(void *instance);
	void (*on_destroy_entity)(void *instance, struct Entity);

	void (*save)(void *instance, SerializationContext&);
	void (*load)(void *instance, const SerializationContext&);
};
