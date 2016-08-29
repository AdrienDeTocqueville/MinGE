#include "Components/Script.h"

#include "Systems/ScriptEngine.h"

Script::Script()
{ }

Script::~Script()
{ }

/// Methods (public)
Component* Script::clone()
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
    ScriptEngine::get()->addComponent(this);
}

void Script::deregisterComponent()
{
    ScriptEngine::get()->removeComponent(this);
}
