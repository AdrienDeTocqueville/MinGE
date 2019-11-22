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
		Key(float &&t, vec3 &&p, quat &&r):
			time(t), pos(p), rot(r)
		{}

		float time;
		vec3 pos;
		quat rot;
	};

	typedef std::vector<Key> BoneTrack;

	std::vector<BoneTrack> channels;
};
