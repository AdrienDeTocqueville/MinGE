#include "Components/Graphic.h"
#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"
#include "Renderer/CommandKey.h"

#include "Assets/Program.h"

Graphic::Graphic(MeshRef _mesh)
{
	mesh = _mesh;
	setMesh(_mesh);
}

Graphic::Graphic(MeshRef _mesh, std::vector<MaterialRef> _materials)
{
	mesh = _mesh;
	setMesh(_mesh, _materials);
}

Graphic::Graphic(MeshRef _mesh, std::initializer_list<MaterialRef> _materials)
{
	mesh = _mesh;
	setMesh(_mesh, _materials);
}

Graphic::~Graphic()
{ }

/// Methods (public)
Graphic* Graphic::clone() const
{
	return new Graphic(mesh, materials);
}

void Graphic::render(CommandBucket *bucket) const
{
	for (uint32_t view_id(0) ; view_id < bucket->current_view; ++view_id)
	{
		CommandBucket::View *view = bucket->get_view(view_id);
		if (!(view->passes & (1 << RenderPass::Forward)))
			continue;

		const mat4 &model = tr->getToWorld();

		vec4 pos = view->vp * vec4(tr->position, 1.0f);
		float depth = pos.z / pos.w;

		unsigned vao = mesh->vao;

		for (int i(0); i < materials.size(); i++)
		{
			if (!materials[i]->hasRenderPass(RenderPass::Forward))
				continue;

			Submesh *submesh = mesh->submeshes.data() + i;

			uint64_t key = CommandKey::encode(view_id, RenderPass::Forward, materials[i]->getId(), depth);
			DrawElements *cmd = bucket->add<DrawElements>(key);
			cmd->model = model;
			cmd->vao = vao;
			cmd->mode = submesh->mode;
			cmd->count = submesh->count;
			cmd->offset = submesh->offset;
		}
	}
}

/// Setters
void Graphic::setMesh(MeshRef _mesh)
{
	updateMesh(_mesh);

	if (mesh == nullptr)
		return;

	for (int i(0); i < mesh->submeshes.size(); i++)
		materials.emplace_back(Material::getDefault());
}

void Graphic::setMesh(MeshRef _mesh, std::vector<MaterialRef> _materials)
{
	updateMesh(_mesh);

	if (mesh == nullptr)
		return;

	if (_materials.size() != mesh->submeshes.size())
		Error::add(USER_ERROR, "Wrong material count");

	for (auto mat : _materials)
		materials.emplace_back(mat);
}

void Graphic::setMesh(MeshRef _mesh, std::initializer_list<MaterialRef> _materials)
{
	updateMesh(_mesh);

	if (mesh == nullptr)
		return;

	if (_materials.size() != mesh->submeshes.size())
		Error::add(USER_ERROR, "Wrong material count");

	for (auto mat : _materials)
		materials.emplace_back(mat);
}

/// Getters
MeshRef Graphic::getMesh() const
{
	return mesh;
}

AABB Graphic::getAABB() const
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

const std::vector<MaterialRef>& Graphic::getMaterials() const
{
	return materials;
}

/// Methods (private)
void Graphic::updateMesh(MeshRef _mesh)
{
	if (mesh && !_mesh)
		GraphicEngine::get()->removeGraphic(this);
	else if (!mesh && _mesh)
		GraphicEngine::get()->addGraphic(this);
	mesh = _mesh;
	materials.clear();
}

void Graphic::onRegister()
{
	/*
	Animator* a = find<Animator>();
	if (a != nullptr)
		a->setGraphic(this);
	*/

	if (mesh)
		GraphicEngine::get()->addGraphic(this);
}

void Graphic::onDeregister()
{
	/*
	Animator* a = find<Animator>();
	if (a != nullptr)
		a->setGraphic(nullptr);
	*/

	if (mesh)
		GraphicEngine::get()->removeGraphic(this);
}
