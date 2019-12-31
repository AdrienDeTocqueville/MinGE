#include "Components/Graphic.h"
#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"

#include "Renderer/RenderContext.inl"
#include "Renderer/CommandKey.h"
#include "Renderer/Commands.h"

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

void Graphic::render(RenderContext *ctx, uint32_t num_views, View const *views) const
{
	const mat4 &model = tr->getToWorld();

	unsigned vao = mesh->getVAO();
	const Submesh *submeshes = mesh->getSubmeshes().data();

	std::vector<void*> commands(materials.size());
	for (size_t i(0); i < materials.size(); i++)
	{
		auto *cmd = ctx->create<DrawElements>();
		commands[i] = cmd;

		cmd->model = model;
		cmd->vao = vao;
		memcpy(&(cmd->submesh), submeshes + i, sizeof(Submesh));
	}

	for (uint32_t view_id(0) ; view_id < num_views; ++view_id)
	{
		const View &view = views[view_id];

		vec4 pos = view.vp * vec4(tr->position, 1.0f);
		float depth = pos.z / pos.w;

		for (size_t i(0); i < materials.size(); i++)
		{
			if (!materials[i]->has_pass(view.pass))
				continue;

			uint64_t key = CommandKey::encode(view_id, view.pass, materials[i]->get_id(), depth);
			ctx->add(key, commands[i]);
		}
	}
}

/// Setters
void Graphic::setMesh(MeshRef _mesh)
{
	updateMesh(_mesh);

	if (mesh == nullptr)
		return;

	for (size_t i(0); i < mesh->getSubmeshes().size(); i++)
		materials.emplace_back(Material::getDefault());
}

void Graphic::setMesh(MeshRef _mesh, std::vector<MaterialRef> _materials)
{
	updateMesh(_mesh);

	if (mesh == nullptr)
		return;

	if (_materials.size() != mesh->getSubmeshes().size())
		Error::add(Error::USER, "Wrong material count");

	materials = _materials;
}

void Graphic::setMesh(MeshRef _mesh, std::initializer_list<MaterialRef> _materials)
{
	updateMesh(_mesh);

	if (mesh == nullptr)
		return;

	if (_materials.size() != mesh->getSubmeshes().size())
		Error::add(Error::USER, "Wrong material count");

	materials = _materials;
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
	if (mesh)
		GraphicEngine::get()->addGraphic(this);
}

void Graphic::onDeregister()
{
	if (mesh)
		GraphicEngine::get()->removeGraphic(this);
}
