#include "Components/Animator.h"

#include "Components/Graphic.h"
#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"
#include "Utility/Time.h"

static bool isnan(const mat4 &m)
{
	for (unsigned i(0) ; i < 4 ; i++)
	for (unsigned j(0) ; j < 4 ; j++)
		if (std::isnan(m[i][j]))
			return true;
	return false;
}

static void get_bones(Transform *t, Transform **bones, int &offset)
{
	for (Transform *c : t->getChildren())
	{
		bones[offset++] = c;
		get_bones(c, bones, offset);
	}
}

Animator::Animator(Skeleton _skeleton, std::vector<AnimationRef> _animations):
	skeleton(_skeleton), animations(_animations),
	matrices(NULL), anim(-1)
{
	bones = new Transform*[skeleton.offsets.size()];
	matrices = new mat4[skeleton.offsets.size()];
}

Animator::~Animator()
{
	delete[] matrices;
	delete[] bones;
}

/// Methods (public)
Animator* Animator::clone() const
{
	return new Animator(skeleton, animations);
}

void Animator::addAnimation(AnimationRef animation)
{
	animations.push_back(animation);
}

void Animator::play(int index)
{
	if (index == -1 || index >= animations.size())
	{
		anim = -1;
		return;
	}

	anim = index;

	keyframes.clear();
	keyframes.resize(animations[anim]->channels.size());
}

void Animator::animate()
{
	if (anim == -1)
		return;

	for (size_t i(0) ; i < keyframes.size(); i++)
	{
		Animation::Track &track = animations[anim]->channels[i];

		Transform *t = bones[track.bone_index];

		while ((keyframes[i].frame + 1 < track.keys.size()) &&
			(keyframes[i].time >= track.keys[keyframes[i].frame + 1].time))
			keyframes[i].frame++;

		if (keyframes[i].frame + 1 == track.keys.size())
		{
			Animation::Key &last = track.keys[keyframes[i].frame];
			if (track.loop)
			{
				keyframes[i].time -= last.time;
				keyframes[i].frame = 0;
			}
			if (!track.loop || track.keys.size() == 1)
			{
				t->position = last.pos;
				t->rotation = last.rot;
				continue;
			}
		}

		size_t curr_key = keyframes[i].frame;

		Animation::Key &curr = track.keys[curr_key];
		Animation::Key &next = track.keys[curr_key + 1];

		float alpha = (keyframes[i].time - curr.time) / (next.time - curr.time);
		keyframes[i].time += Time::deltaTime;

		t->position = mix(curr.pos, next.pos, alpha);
		t->rotation = slerp(curr.rot, next.rot, alpha);
	}

	upload();
}

/// Getter
Transform* Animator::getBone(unsigned index) const
{
	return bones[index];
}

Transform* Animator::getBone(std::string name) const
{
	auto it = skeleton.bone_index.find(name);
	if (it != skeleton.bone_index.end())
		return bones[it->second];

	return nullptr;
}

const Skeleton &Animator::getSkeleton() const
{
	return skeleton;
}

const std::vector<AnimationRef> &Animator::getAnimations() const
{
	return animations;
}

/// Private
void Animator::onRegister()
{
	int offset = 0;
	get_bones(tr, bones, offset);

	if (Graphic *g = find<Graphic>())
	{
		upload();
		GraphicEngine::get()->addAnimator(this);
	}
	else
		Error::add(USER_ERROR, "Add a graphic before animator");
}

void Animator::upload()
{
	tr->toMatrix(); // Updates all hierarchy
	for (int i=0; i < skeleton.offsets.size(); i++)
	{
		simd_mul(bones[i]->getToWorld(), skeleton.offsets[i], matrices[i]);
		simd_mul(tr->getToLocal(), matrices[i], matrices[i]);

		// * offset : transform to bone space
		// * bones[i]->world : transform to root world space
		// * tr->local : transform to entity local space so that
		//		 if entity moves, the model can be moved without
		//		 recomputing all bones
	}

	if (Graphic *g = find<Graphic>())
	{
		for (MaterialRef mat : g->getMaterials())
			mat->set("bones", matrices, skeleton.offsets.size());
	}
}
