#include "Components/Graphic.h"
#include "Components/Animator.h"
#include "Components/Transform.h"

Graphic::Graphic(Mesh* _mesh)
{
	setMesh(_mesh);
}

Graphic::Graphic(Mesh* _mesh, std::vector<MaterialRef> _materials)
{
	setMesh(_mesh, _materials);
}

Graphic::~Graphic()
{
}

/// Methods (public)
Graphic* Graphic::clone() const
{
	return new Graphic(mesh, materials);
}

void Graphic::render()
{
	if (mesh == nullptr)
		return;

	GL::BindVertexArray(mesh->vao);
	for (int i(0); i < materials.size(); i++)
	{
		const MaterialRef& material = materials[i];
		const Submesh& submesh = mesh->submeshes[i];

		tr->use();
		material->bind();
		material->set("MATRIX_M", tr->toWorldSpace);
		material->set("MATRIX_N", tr->toWorldSpace);
		submesh.draw();
	}
}

/// Setters
void Graphic::setMesh(Mesh* _mesh)
{
	mesh = _mesh;
	materials.clear();

	if (mesh == nullptr)
		return;

	for (int i(0); i < mesh->submeshes.size(); i++)
		materials.emplace_back(Material::getDefault());
}

void Graphic::setMesh(Mesh* _mesh, std::vector<MaterialRef> _materials)
{
	mesh = _mesh;
	materials.clear();

	if (mesh == nullptr)
		return;

	if (_materials.size() != mesh->submeshes.size())
		Error::add(USER_ERROR, "Wrong material count");

	for (auto mat : _materials)
		materials.emplace_back(mat);
}

/// Getters
Mesh* Graphic::getMesh() const
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
	vec3 lowest = tr->getToWorldSpace(center - hdim);

	hdim.z *= -1;
	lowest = min(lowest, tr->getToWorldSpace(center - hdim));

	// les 6 autres points du cube
	for (int i: {0, 1, 0})
	{
		hdim[i] *= -1;

		lowest = min(lowest, tr->getToWorldSpace(center - hdim));
		hdim.z *= -1;
		lowest = min(lowest, tr->getToWorldSpace(center - hdim));
	}

	center = tr->getToWorldSpace(center);

	box.bounds[0] = lowest;
	box.bounds[1] = center + center - lowest;
	return box;
}

const std::vector<MaterialRef>& Graphic::getMaterials() const
{
	return materials;
}

/// Methods (private)
void Graphic::onRegister()
{
	/*
	Animator* a = find<Animator>();
	if (a != nullptr)
		a->setGraphic(this);
	*/

	GraphicEngine::get()->addGraphic(this);
}

void Graphic::onDeregister()
{
	/*
	Animator* a = find<Animator>();
	if (a != nullptr)
		a->setGraphic(nullptr);
	*/

	GraphicEngine::get()->removeGraphic(this);
}
