#pragma once

#include "Components/Component.h"
#include "Assets/Animation.h"

struct Skeleton
{
	std::unordered_map<std::string, unsigned> bone_index;
	std::vector<mat4> offsets;
};

class Animator : public Component
{
public:
	Animator(Skeleton _skeleton, std::vector<AnimationRef> _animations);
	virtual ~Animator();

	/// Methods (public)
	virtual Animator* clone() const override;

	void addAnimation(AnimationRef animation);

	void play(int index);
	void animate();

	/// Getter
	Transform* getBone(unsigned index) const;
	Transform* getBone(std::string name) const;

	const Skeleton &getSkeleton() const;
	const std::vector<AnimationRef> &getAnimations() const;

private:
	/// Methods (private)
	virtual void onRegister() override;
	void upload();

	/// Attributes (private)
	Skeleton skeleton;
	std::vector<AnimationRef> animations;

	Transform **bones;
	mat4 *matrices;

	size_t anim;
	float accumulator;

	std::vector<size_t> keyframes;
};
