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

Animator::Animator(Skeleton _skeleton):
	skeleton(_skeleton)
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
	return new Animator(skeleton);
}

void Animator::setMotion(vec2 _pos)
{
	static const float threshold = 0.01f;
	static const float eps = 1.0f;

	if (epsilonEqual(pos, _pos))
		return;
	pos = _pos;

	float total_weights = 0.0f;
	for (auto &motion : blender.motions)
	{
		float d = length(pos - motion.position);
		motion.weight = 1.0f - eps * d;

		if (motion.weight < threshold)
			motion.weight = 0.0f;

		total_weights += motion.weight;
	}

	float scale = (total_weights == 0.0f) ? 0.0f : 1.0f / total_weights;
	for (auto &motion : blender.motions)
		motion.weight *= scale;
}

void Animator::animate()
{
	for (auto &motion : blender.motions)
		motion.update();

	for (size_t i = 0; i < skeleton.offsets.size(); i++)
	{
		bones[i]->position = vec3(0.0f);
		bones[i]->rotation = quat(0,0,0,0);

		for (const auto &motion : blender.motions)
		{
			if (motion.weight == 0.0f)
				continue;

			bones[i]->position += motion.pos[i] * motion.weight;

			const float sign = dot(bones[i]->rotation, motion.rot[i]) > 0.0f ? 1.0f : -1.0f;
			bones[i]->rotation += motion.rot[i] * sign * motion.weight;
		}
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

void Animator::setMotionBlender(MotionBlender &&_blender)
{
	blender = std::move(_blender);

	for (auto &m : blender.motions)
	{
		m.time = 0.0f;
		m.pos.resize(skeleton.offsets.size());
		m.rot.resize(skeleton.offsets.size());
		for (size_t i = 0; i < skeleton.offsets.size(); i++)
		{
			m.pos[i] = bones[i]->position;
			m.rot[i] = bones[i]->rotation;
		}
	}

	pos = vec2(-1.0f);
	setMotion(vec2(0.0f));
}

/// Private
void Animator::onRegister()
{
	int offset = 0;
	get_bones(tr, bones, offset);

	Graphic *g = find<Graphic>();
	if (g == NULL)
		return Error::add(Error::USER, "Animator requires a Graphic component.");

	// Enable skinning
	std::vector<MaterialRef> &materials = g->getMaterials();
	for (int i(0); i < materials.size(); i++)
	{
		if (!materials[i]->ifdef("SKINNED"))
		{
			materials[i] = materials[i]->clone();
			materials[i]->define("SKINNED");
		}
	}

	upload();
	GraphicEngine::get()->addAnimator(this);
}

void Animator::upload()
{
	tr->toMatrix(); // Updates all hierarchy
	for (size_t i(0); i < skeleton.offsets.size(); i++)
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
		for (MaterialRef m: g->getMaterials())
			m->set("bones", matrices, skeleton.offsets.size());
	}
}

bool MotionBlender::BoneFrame::advance(const Animation::Track &track, float time)
{
	while ((frame + 1 < track.keys.size()) &&
		(time >= track.keys[frame + 1].time))
		frame++;

	// If we reached end of track
	if (frame + 1 == track.keys.size())
		return false;

	return true;
}

void MotionBlender::Motion::update()
{
	if (weight != 0.0f)
	for (size_t i(0) ; i < keyframes.size(); i++)
	{
		const Animation::Track &track = anim->channels[i];

		if (!keyframes[i].advance(track, time))
		{
			const Animation::Key &last = track.keys.back();
			pos[track.bone_index] = last.pos;
			rot[track.bone_index] = last.rot;
			continue;
		}

		const size_t curr_key = keyframes[i].frame;

		const Animation::Key &curr = track.keys[curr_key];
		const Animation::Key &next = track.keys[curr_key + 1];

		const float alpha = (time - curr.time) / (next.time - curr.time);

		pos[track.bone_index] = mix(curr.pos, next.pos, alpha);
		rot[track.bone_index] = slerp(curr.rot, next.rot, alpha);
	}

	time += Time::deltaTime;
	if (time >= anim->duration)
	{
		time -= anim->duration;
		memset(keyframes.data(), 0, keyframes.size() * sizeof(BoneFrame));
	}
}
