#pragma once

#include <string>

#include "Entity.h"
#include "Assets/Animation.h"

struct Scene
{
	static Entity *import(const std::string &file);
	static AnimationRef import_animation(const std::string &file, const struct Skeleton &skeleton);
};
