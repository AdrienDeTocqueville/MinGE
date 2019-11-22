#pragma once

#include "Components/Component.h"
#include "Assets/Animation.h"

struct Skeleton
{
	std::unordered_map<std::string, unsigned> bone_index;
	std::vector<mat4> offsets;
	std::vector<Transform*> nodes;
};

class Animator : public Component
{
public:
	Animator(Skeleton _skeleton, std::vector<AnimationRef> _animations);
	virtual ~Animator();

	/// Methods (public)
	virtual Animator* clone() const override;

	void play(int _index);

	void animate();

	/// Getter
	Transform* getBone(unsigned _index);
	Transform* getBone(std::string _name);

private:
	/// Methods (private)
	virtual void onRegister() override;

	/// Attributes (private)
	Skeleton skeleton;
	std::vector<AnimationRef> animations;

	mat4 *matrices;
	size_t current;
	/*
	float accumulator;

	std::vector<unsigned> trKeys;
	std::vector<unsigned> roKeys;

	std::vector<Transform*> bones;

	int root;
	*/
};
