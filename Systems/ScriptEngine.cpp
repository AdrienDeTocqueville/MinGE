#include "Systems/ScriptEngine.h"

#include "Components/Script.h"
#include "Entity.h"

ScriptEngine* ScriptEngine::instance = nullptr;

/// Methods (private)
ScriptEngine::ScriptEngine()
{ }

ScriptEngine::~ScriptEngine()
{ }

void ScriptEngine::clear()
{
    components.clear();
}

/// Methods (static)
void ScriptEngine::create()
{
    if (instance != nullptr)
        return;

    instance = new ScriptEngine();
}

void ScriptEngine::destroy()
{
    delete instance;
    instance = nullptr;
}

ScriptEngine* ScriptEngine::get()
{
    return instance;
}

/// Methods (public)
void ScriptEngine::addComponent(Script* _script)
{
    started.push_back(_script);
    components.push_back(_script);
}

void ScriptEngine::removeComponent(const Script* _script)
{
    for (unsigned i(0) ; i < components.size() ; i++)
    {
        if (components[i] == _script)
        {
            components[i] = components.back();
            components.pop_back();

            return;
        }
    }
}

void ScriptEngine::start()
{
    for (unsigned i(0) ; i < started.size() ; i++)
        started[i]->start();
    started.clear();
}

void ScriptEngine::update()
{
    for (unsigned i(0) ; i < components.size() ; i++)
        components[i]->update();
}

void ScriptEngine::lateUpdate() const
{
    for (unsigned i(0) ; i < components.size() ; i++)
        components[i]->lateUpdate();
}
