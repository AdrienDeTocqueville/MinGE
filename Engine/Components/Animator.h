#pragma once

#include "Components/Component.h"
#include "Assets/Animation.h"

struct Skeleton
{
	std::unordered_map<std::string, unsigned> bone_index;
	std::vector<mat4> offsets;
};

struct MotionBlender
{
	struct BoneFrame
	{
		size_t frame;
		bool advance(const Animation::Track &track, float time);
	};

	struct Motion
	{
		Motion(vec2 _pos, AnimationRef _anim): position(_pos), anim(_anim)
		{ keyframes.resize(anim->channels.size()); }

		float time, weight;
		vec2 position;
		AnimationRef anim;

		std::vector<BoneFrame> keyframes;
		std::vector<vec3> pos;
		std::vector<quat> rot;

		void update();
	};

	MotionBlender() {}
	MotionBlender(std::initializer_list<Motion> &&_motions): motions(_motions) {}

	MotionBlender& operator=(MotionBlender &&x) { std::swap(motions, x.motions); return *this; }

	std::vector<Motion> motions;
};

class Animator : public Component
{
public:
	Animator(Skeleton _skeleton);
	virtual ~Animator();

	/// Methods (public)
	virtual Animator* clone() const override;

	void setMotion(vec2 _pos);
	void animate();

	/// Getters
	Transform* getBone(unsigned index) const;
	Transform* getBone(std::string name) const;

	const Skeleton &getSkeleton() const;

	/// Setters
	void setMotionBlender(MotionBlender &&_blender);

private:
	/// Methods (private)
	virtual void onRegister() override;
	void upload();

	/// Attributes (private)
	Skeleton skeleton;
	MotionBlender blender;
	vec2 pos;

	Transform **bones;
	mat4 *matrices;
};
