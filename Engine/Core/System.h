#pragma once

#include <cstdint>
#include <fstream>

#include "IO/json_fwd.hpp"

struct system_type_t
{
	const char *name;
	uint32_t size;
	uint32_t on_main_thread; // bool

	void (*init)(void *instance);
	void (*destroy)(void *instance);
	void (*update)(void *instance);

	nlohmann::json (*serialize)(void *instance);
	void (*deserialize)(const nlohmann::json&);
};
