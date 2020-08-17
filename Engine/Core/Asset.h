#pragma once

#include <stdint.h>

#include "IO/json_fwd.hpp"

struct SerializationContext;
struct asset_type_t
{
	const char *name;

	void (*save)(nlohmann::json&);
	void (*load)(const nlohmann::json&);

	void (*clear)();
};
