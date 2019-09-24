#include "Components/Graphic.h"
#include "Components/Animator.h"
#include "Components/Transform.h"

Graphic::Graphic(Mesh* _mesh)
{
	setMesh(_mesh);
}

Graphic::~Graphic()
{
	for (Material* material: materials)
		delete material;

	materials.clear();
}

/// Methods (public)
Graphic* Graphic::clone() const
{
	return new Graphic(mesh);
}

void Graphic::render()
{
	if (mesh != nullptr)
		mesh->render(tr, materials);
}

/// Setters
void Graphic::setMesh(Mesh* _mesh)
{
	mesh = _mesh;

	if (mesh == nullptr)
		return;

	for (Material* material: mesh->materials)
		materials.push_back(material->clone());
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

Material* Graphic::getMaterial(unsigned _index) const
{
	return materials[_index];
}

const std::vector<Material*>& Graphic::getMaterials() const
{
	return materials;
}

/// Methods (private)
void Graphic::onRegister()
{
	Animator* a = find<Animator>();
	if (a != nullptr)
		a->setGraphic(this);

	GraphicEngine::get()->addGraphic(this);
}

void Graphic::onDeregister()
{
	Animator* a = find<Animator>();
	if (a != nullptr)
		a->setGraphic(nullptr);

	GraphicEngine::get()->removeGraphic(this);
}
