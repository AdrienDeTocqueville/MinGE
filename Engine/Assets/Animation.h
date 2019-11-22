#pragma once

#include <vector>
#include <memory>

#include "Utility/glm.h"

typedef std::shared_ptr<class Animation> AnimationRef;

class Animation
{
public:
	Animation(std::string _name, float _duration, bool _loop):
		name(_name), duration(_duration), loop(_loop)
	{ }

	std::string name;
	float duration;
	bool loop;


	struct Key
	{
		Key(float &&t, vec3 &&p, quat &&r):
			time(t), pos(p), rot(r)
		{}

		float time;
		vec3 pos;
		quat rot;
	};

	struct Track
	{
		Track(size_t i, size_t n):
			bone_index(i)
		{ keys.reserve(n); }

		size_t bone_index;
		std::vector<Key> keys;
	};

	std::vector<Track> channels;
};
