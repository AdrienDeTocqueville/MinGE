#pragma once

#include <cstdint>
#include <fstream>

#include "IO/json_fwd.hpp"

#define SYSTEM_TYPE(sys, dependency_count) { #sys, sizeof(sys), dependency_count,\
	sys::init,\
	sys::destroy,\
	sys::update,\
	sys::serialize,\
	sys::deserialize\
}

struct system_type_t
{
	const char *name;
	uint32_t size, dependency_count;

	void (*init)(void *instance);
	void (*destroy)(void *instance);
	void (*update)(void *instance);

	nlohmann::json (*serialize)(void *instance);
	void (*deserialize)(const nlohmann::json&);
};
