#include "Components/Component.h"

float Component::deltaTime = 0.0f;
unsigned Component::instances = 0;

Component::Component():
    entity(nullptr), tr(nullptr)
{
    instances++;
}

Component::~Component()
{
    instances--;
}

void Component::attach(Entity* _entity)
{
    std::vector<Component*>& vec = _entity->components[ typeid(*this).hash_code() ];

    if (vec.size())
    {
        Error::add(MINGE_ERROR, "You can only have one component of this type per entity: " + std::string(typeid(*this).name()));
        return;
    }

    detach();   // Quit actual entity

    entity = _entity;
    tr = entity->getTransform();

    vec.push_back(this);
}

void Component::detach()
{
    if (entity == nullptr)
        return;

    size_t hashCode = typeid(*this).hash_code();
    auto& vec = entity->components[hashCode];

    for (unsigned i(0) ; i < vec.size() ; i++)
        if (vec[i] == this)
        {
            vec.erase( vec.begin() + i );
            break;
        }

    entity = nullptr;
}

void Component::destroy()
{
    detach();
    deregisterComponent();

    delete this;
}
