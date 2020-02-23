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

void Script::onCollision(const Collision&) {}
void Script::onTrigger  (Collider*) {}

/// Methods (private)
void Script::onRegister()
{
	ScriptEngine::get()->addComponent(this);
}

void Script::onDeregister()
{
	destroy();
	ScriptEngine::get()->removeComponent(this);
}
