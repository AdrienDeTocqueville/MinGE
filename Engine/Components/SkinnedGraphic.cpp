#include "Components/SkinnedGraphic.h"
#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"
#include "Renderer/CommandKey.h"

#include <assimp/scene.h>

SkinnedGraphic::SkinnedGraphic(Skeleton _skeleton, MeshRef _mesh,
	std::vector<MaterialRef> _materials, std::vector<AnimationRef> _animations):
	skeleton(_skeleton), mesh(_mesh), animations(_animations)
{
	if (_materials.size() != mesh->getSubmeshes().size())
		Error::add(USER_ERROR, "Wrong material count");

	std::vector<mat4> matrices;
	for (int i=0; i < skeleton.nodes.size(); i++)
		matrices.push_back(skeleton.nodes[i]->getToWorld() * skeleton.offsets[i]);

	materials.reserve(_materials.size());
	for (auto mat : _materials)
	{
		materials.emplace_back(mat);
		mat->set("bones", matrices.data(), matrices.size());
	}
}

SkinnedGraphic::~SkinnedGraphic()
{ }

/// Methods (public)
SkinnedGraphic* SkinnedGraphic::clone() const
{
	return new SkinnedGraphic(skeleton, mesh, materials, animations);
}

void SkinnedGraphic::animate()
{
}

void SkinnedGraphic::render(CommandBucket *bucket) const
{
	for (uint32_t view_id(0) ; view_id < bucket->current_view; ++view_id)
	{
		CommandBucket::View *view = bucket->get_view(view_id);
		if (!(view->passes & (1 << RenderPass::Forward)))
			continue;

		const mat4 &model = tr->getToWorld();

		vec4 pos = view->vp * vec4(tr->position, 1.0f);
		float depth = pos.z / pos.w;

		unsigned vao = mesh->getVAO();
		const Submesh *submeshes = mesh->getSubmeshes().data();

		for (int i(0); i < materials.size(); i++)
		{
			if (!materials[i]->hasRenderPass(RenderPass::Forward))
				continue;

			uint64_t key = CommandKey::encode(view_id, RenderPass::Forward, materials[i]->getId(), depth);
			DrawElements *cmd = bucket->add<DrawElements>(key);
			cmd->model = model;
			cmd->vao = vao;
			memcpy(&(cmd->submesh), submeshes + i, sizeof(Submesh));
		}
	}
}

/// Getters
MeshRef SkinnedGraphic::getMesh() const
{
	return mesh;
}

AABB SkinnedGraphic::getAABB() const
{
	AABB box = mesh->getAABB();
	vec3 center = box.center();
	vec3 hdim = 0.5f * box.dim();

	// On cherche le point le plus bas
	// 1ere iteration
	vec3 lowest = tr->toWorld(center - hdim);

	hdim.z *= -1;
	lowest = min(lowest, tr->toWorld(center - hdim));

	// les 6 autres points du cube
	for (int i: {0, 1, 0})
	{
		hdim[i] *= -1;

		lowest = min(lowest, tr->toWorld(center - hdim));
		hdim.z *= -1;
		lowest = min(lowest, tr->toWorld(center - hdim));
	}

	center = tr->toWorld(center);

	box.bounds[0] = lowest;
	box.bounds[1] = center + center - lowest;
	return box;
}

const std::vector<MaterialRef>& SkinnedGraphic::getMaterials() const
{
	return materials;
}

const std::vector<AnimationRef>& SkinnedGraphic::getAnimations() const
{
	return animations;
}

/// Methods (private)
void SkinnedGraphic::onRegister()
{
	if (mesh)
		GraphicEngine::get()->addSkinnedGraphic(this);
}

void SkinnedGraphic::onDeregister()
{
	if (mesh)
		GraphicEngine::get()->removeSkinnedGraphic(this);
}
