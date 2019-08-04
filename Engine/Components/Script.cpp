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

void Script::start	 () {}
void Script::update	() {}
void Script::lateUpdate() {}
void Script::onDestroy () {}

void Script::onCollision(const Collision&) {}
void Script::onTrigger  (Collider*) {}

/// Methods (private)
void Script::onRegister()
{
	ScriptEngine::get()->addComponent(this);
}

void Script::onDeregister()
{
	onDestroy();
	ScriptEngine::get()->removeComponent(this);
}
