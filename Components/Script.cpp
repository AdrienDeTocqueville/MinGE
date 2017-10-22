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

/// Methods (private)
void Script::onRegister()
{
    ScriptEngine::get()->addComponent(this);
}

void Script::onDeregister()
{
    ScriptEngine::get()->removeComponent(this);
}
