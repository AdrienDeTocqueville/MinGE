#include "Components/Graphic.h"
#include "Components/Transform.h"

Graphic::Graphic(Mesh* _mesh):
    mesh(_mesh)
{ }

Graphic::~Graphic()
{ }

/// Methods (public)
Component* Graphic::clone()
{
    return new Graphic(mesh);
}

void Graphic::registerComponent()
{
    GraphicEngine::get()->addGraphic(this);
}

void Graphic::deregisterComponent()
{
    GraphicEngine::get()->removeGraphic(this);
}

void Graphic::render()
{
    if (mesh != nullptr)
        mesh->render(tr);
}

/// Setters
void Graphic::setMesh(Mesh* _mesh)
{
    mesh = _mesh;
}

/// Getters
Mesh* Graphic::getMesh() const
{
    return mesh;
}
