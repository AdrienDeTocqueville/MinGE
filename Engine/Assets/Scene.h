#pragma once

#include <string>
#include "Entity.h"

struct Scene
{
	static Entity *import(const std::string &file);
};
