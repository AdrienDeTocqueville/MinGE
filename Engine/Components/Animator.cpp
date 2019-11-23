#include "Components/Animator.h"

#include "Components/Graphic.h"
#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"
#include "Utility/Time.h"

static void get_bones(Transform *t, Transform **bones, int &offset)
{
	for (Transform *c : t->getChildren())
	{
		bones[offset++] = c;
		get_bones(c, bones, offset);
	}
}

Animator::Animator(Skeleton _skeleton, std::vector<AnimationRef> _animations):
	skeleton(_skeleton), animations(_animations)
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
		return;

	motions[0].reset(animations[1].get());
	motions[1].reset(animations[2].get());
}

void Animator::animate()
{
	if (!motions[0].anim || !motions[1].anim)
		return;

	for (int i(0); i < skeleton.offsets.size(); i++)
	{
		bones[i]->position = vec3(0.0f);
		bones[i]->rotation = glm::identity<quat>();
	}

	motions[0].update(bones, 1.0f);
	//motions[1].update(bones, 0.5f);

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
		//simd_mul(bones[0]->getToLocal(), matrices[i], matrices[i]);
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

bool Animator::BoneFrame::advance(const Animation::Track &track)
{
	while ((frame + 1 < track.keys.size()) &&
		(time >= track.keys[frame + 1].time))
		frame++;

	// If we reached end of track
	if (frame + 1 == track.keys.size())
	{
		if (!track.loop || track.keys.size() == 1)
			return false;

		// Loop
		time -= track.keys.back().time;
		frame = 0;
	}
	return true;
}

void Animator::Motion::reset(Animation *a)
{
	anim = a;
	keyframes.clear();
	keyframes.resize(a->channels.size());
}

void Animator::Motion::update(Transform **bones, float weight)
{
	for (size_t i(0) ; i < keyframes.size(); i++)
	{
		const Animation::Track &track = anim->channels[i];
		Transform *t = bones[track.bone_index];

		if (!keyframes[i].advance(track))
		{
			const Animation::Key &last = track.keys.back();
			t->position = last.pos;
			t->rotation = last.rot;
			continue;
		}

		const size_t curr_key = keyframes[i].frame;

		const Animation::Key &curr = track.keys[curr_key];
		const Animation::Key &next = track.keys[curr_key + 1];

		const float alpha = (keyframes[i].time - curr.time) / (next.time - curr.time);
		keyframes[i].time += Time::deltaTime;

		t->position += weight * mix(curr.pos, next.pos, alpha);
		t->rotation += weight * slerp(curr.rot, next.rot, alpha);
	}
}
