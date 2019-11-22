#include "Components/Animator.h"

#include "Components/Graphic.h"
#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"
#include "Utility/Time.h"

Animator::Animator(Skeleton _skeleton, std::vector<AnimationRef> _animations):
	skeleton(_skeleton), animations(_animations),
	matrices(NULL), current(-1)
{
	// Init in bind pose
	matrices = new mat4[skeleton.nodes.size()];
	for (int i=0; i < skeleton.nodes.size(); i++)
		simd_mul(skeleton.nodes[i]->getToWorld(), skeleton.offsets[i], matrices[i]);
}

Animator::~Animator()
{
	delete[] matrices;
}

/// Methods (public)
Animator* Animator::clone() const
{
	return new Animator(skeleton, animations);
}

void Animator::play(int _index)
{
	current = _index;
	/*
	current = &(model->animations[_index]);
	accumulator = 0.0f;
	loop = _repeat;

	trKeys.clear(); roKeys.clear();
	trKeys.resize(current->tracks.size(), 0);
	roKeys.resize(current->tracks.size(), 0);
	*/
}

void Animator::animate()
{
	if (current == -1)
		return;

	for (Animation::Track &track : animations[current]->channels)
	{
		Transform *t = skeleton.nodes[track.bone_index];
		t->position = track.keys[0].pos;
		t->rotation = track.keys[0].rot;
	}

	tr->toMatrix(); // Updates all hierarchy
	for (int i=0; i < skeleton.nodes.size(); i++)
		simd_mul(skeleton.nodes[i]->getToWorld(), skeleton.offsets[i], matrices[i]);

	if (Graphic *g = find<Graphic>())
	{
		for (auto mat : g->getMaterials())
			mat->set("bones", matrices, skeleton.nodes.size());
	}

	/*
	accumulator += Time::deltaTime;

	if (accumulator >= current->duration)
	{
		if (loop)
		{
			trKeys.clear(); roKeys.clear();
			trKeys.resize(current->tracks.size(), 0);
			roKeys.resize(current->tracks.size(), 0);

			accumulator -= current->duration;
		}
		else
		{
			current = nullptr;
			return;
		}
	}

	for (unsigned t(0) ; t < current->tracks.size() ; t++)
	{
		AnimatedModel::Track& track = current->tracks[t];
		unsigned id = track.boneId;

		// Translations
		auto& trs = track.translations;

		for (; trKeys[t] < trs.size() ; trKeys[t]++)
			if (trs[trKeys[t]].first >= accumulator)
				break;

		float coefT = (accumulator-trs[trKeys[t]-1].first) / (trs[trKeys[t]].first-trs[trKeys[t]-1].first);
		bones[id]->position = model->bones[id].position + mix(trs[trKeys[t]-1].second, trs[trKeys[t]].second, coefT);

		// Rotations
		auto& ros = track.rotations;

		for (; roKeys[t] < ros.size() ; roKeys[t]++)
			if (ros[roKeys[t]].first >= accumulator)
				break;

		float coefR = (accumulator-ros[roKeys[t]-1].first) / (ros[roKeys[t]].first-ros[roKeys[t]-1].first);
		bones[id]->rotation = model->bones[id].rotation * mix(ros[roKeys[t]-1].second, ros[roKeys[t]].second, coefR);
	}

	tr->toMatrix();


	for (unsigned i(0) ; i < bones.size() ; i++)
		matrices[i] = bones[i]->toWorldSpace * model->bones[i].inverseBindMatrix;
	*/
}

/// Getter
Transform* Animator::getBone(unsigned _index)
{
	return skeleton.nodes[_index];
}

Transform* Animator::getBone(std::string _name)
{
	auto it = skeleton.bone_index.find(_name);
	if (it != skeleton.bone_index.end())
		return skeleton.nodes[it->second];

	return nullptr;
}

/// Private
void Animator::onRegister()
{
	if (Graphic *g = find<Graphic>())
	{
		for (auto mat : g->getMaterials())
			mat->set("bones", matrices, skeleton.nodes.size());

		GraphicEngine::get()->addAnimator(this);
	}
	else
		Error::add(USER_ERROR, "Add a graphic before animator");
}
