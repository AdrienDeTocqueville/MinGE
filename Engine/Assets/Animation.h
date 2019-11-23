#pragma once

#include <vector>
#include <memory>

#include "Utility/glm.h"

typedef std::shared_ptr<class Animation> AnimationRef;

class Animation
{
public:
	Animation(std::string _name, float _duration):
		name(_name), duration(_duration)
	{ }

	std::string name;
	float duration;


	struct Key
	{
		Key(float t, vec3 p, quat r):
			time(t), pos(p), rot(r)
		{}

		float time;
		vec3 pos;
		quat rot;
	};

	struct Track
	{
		Track(size_t i, size_t n, bool l):
			bone_index(i), loop(l)
		{ keys.reserve(n); }

		size_t bone_index;
		std::vector<Key> keys;
		bool loop;
	};

	std::vector<Track> channels;
};
