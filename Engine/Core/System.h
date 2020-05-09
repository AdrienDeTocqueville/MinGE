#pragma once

#include <stdint.h>

#include "IO/json_fwd.hpp"

struct system_type_t
{
	const char *name;
	uint32_t size;

	void (*destroy)(void *instance);
	void (*update)(void *instance);

	nlohmann::json (*serialize)(void *instance);
	void (*deserialize)(void *instance, const nlohmann::json&);
};
