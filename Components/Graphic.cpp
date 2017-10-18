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

void Graphic::attach(Entity* _entity)
{
    Component::attach(_entity);

    Animator* a = getComponent<Animator>();
    if (a != nullptr)
        a->setGraphic(this);
}

void Graphic::detach()
{
    Animator* a = getComponent<Animator>();
    if (a != nullptr)
        a->setGraphic(nullptr);

    Component::detach();
}

void Graphic::registerComponent()
{
    if (registered)
        return;

    GraphicEngine::get()->addGraphic(this);
    registered = true;
}

void Graphic::deregisterComponent()
{
    if (!registered)
        return;

    GraphicEngine::get()->removeGraphic(this);
    registered = false;
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

Material* Graphic::getMaterial(unsigned _index) const
{
    return materials[_index];
}

const std::vector<Material*>& Graphic::getMaterials() const
{
    return materials;
}
