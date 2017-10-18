#include "Components/Script.h"

#include "Systems/ScriptEngine.h"

Script::Script()
{ }

Script::~Script()
{ }

/// Methods (public)
Script* Script::clone() const
{
    return nullptr;
}
void Script::attach(Entity* _entity)
{
    detach();   // Quit actual entity

    entity = _entity;
    tr = entity->getTransform();

    size_t hashCode = typeid(Script).hash_code();
    entity->components[hashCode].push_back(this);
}

void Script::registerComponent()
{
    if (registered)
        return;

    ScriptEngine::get()->addComponent(this);
    registered = true;
}

void Script::deregisterComponent()
{
    if (!registered)
        return;

    ScriptEngine::get()->removeComponent(this);
    registered = false;
}
