#include "MinGE.h"

std::vector< std::string > Entity::tags = { "Untagged", "MainCamera" };
std::list<Entity*> Entity::entities;

Entity::Entity(std::string _tag):
    tag(getTagId(_tag)),
    tr(nullptr)
{
    entities.push_back(this);

    (new Transform())->attach(this);
}

Entity::Entity(std::string _tag, std::vector<Component*> _components, bool _prototype):
    tag(getTagId(_tag)), prototype(_prototype),
    components(),
    tr(nullptr)
{
    entities.push_back(this);

    for (Component* component: _components)
    {
        if (typeid(*component) == typeid(Transform))
        {
            component->attach(this);
            break;
        }
    }

    if (tr == nullptr)
        (new Transform())->attach(this);


    for (Component* component: _components)
        component->attach(this);


    if (!prototype)
    for (auto& componentPair: components)
        for (Component* component: componentPair.second)
            component->registerComponent();
}

Entity::~Entity()
{
    // free components
    for (auto& componentPair: components)
        for (Component* component: componentPair.second)
            component->destroy();
}

/// Methods (public)
Entity* Entity::clone(vec3 _position, vec3 _rotation, vec3 _scale)
{
    std::vector<Component*> _components;

    for (auto& componentPair: components)
        for (Component* component: componentPair.second)
        {
            if (typeid(*component) == typeid(Transform))
                _components.push_back(new Transform(_position, _rotation, _scale));
            else
                _components.push_back(component->clone());
        }


    return new Entity(getTagName(), _components);
}

void Entity::addComponent(Component* _component)
{
    if (_component == nullptr)
        return;

    if (_component->entity != this)
        _component->attach(this);

    if (_component->entity == this && !prototype)
        _component->registerComponent();
}

void Entity::removeComponent(Component* _component)
{
    if (_component == nullptr || _component->entity != this)
        return;

    _component->detach();
    _component->deregisterComponent();
}

void Entity::destroy()
{
    entities.remove(this);

    delete this;
}

/// Methods (static)
unsigned Entity::getTagId(std::string _tag)
{
    auto it = std::find(tags.begin(), tags.end(), _tag);

    if ( it != tags.end() )
       return it - tags.begin();
    else
    {
        tags.push_back(_tag);
        return tags.size()-1;
    }
}

Entity* Entity::findByTag(std::string _tag)
{
    unsigned _tagId;

    auto it = std::find(tags.begin(), tags.end(), _tag);

    if ( it != Entity::tags.end() )
       _tagId = it - Entity::tags.begin();
    else
        return nullptr;

    for (Entity* entity: entities)
    {
        if (entity->tag == _tagId)
            return entity;
    }

    return nullptr;
}

std::list<Entity*> Entity::findAllByTag(std::string _tag)
{
    unsigned _tagId;
    std::list<Entity*> entities;

    auto it = std::find(tags.begin(), tags.end(), _tag);

    if ( it != tags.end() )
       _tagId = it - tags.begin();
    else
        return entities;


    for (Entity* entity: entities)
    {
        if (entity->tag == _tagId)
            entities.push_back(entity);
    }

    return entities;
}

void Entity::clear()
{
    for(Entity* entity: entities)
        delete entity;

    entities.clear();
}

/// Getters
unsigned Entity::getTag() const
{
    return tag;
}

std::string Entity::getTagName() const
{
    return tags[tag];
}

size_t Entity::getColliderHashCode()
{
    return typeid(Collider).hash_code();
}

size_t Entity::getScriptHashCode()
{
    return typeid(Script).hash_code();
}

Transform* Entity::getTransform() const
{
    return tr;
}

template <>
void Entity::removeComponents<Collider>()
{
    for (Component* collider: components[typeid(Collider).hash_code()])
    {
        Collider* c = static_cast<Collider*>(collider);

        c->entity = nullptr;
        c->rigidBody = nullptr;

        c->deregisterComponent();
    }

    components[typeid(Collider).hash_code()].clear();

    RigidBody* rb = getComponent<RigidBody>();
    if (rb != nullptr)
        rb->computeMass();
}

template <>
void Entity::removeComponents<Script>()
{
    for (Component* script: components[typeid(Script).hash_code()])
    {
        Script* s = static_cast<Script*>(script);

        s->entity = nullptr;

        s->deregisterComponent();
    }

    components[typeid(Script).hash_code()].clear();
}

template <>
std::vector<Component*> Entity::getComponents()
{
    std::vector<Component*> _components;

    for (auto& componentPair: components)
        for (Component* component: componentPair.second)
            _components.push_back(component);

    return _components;
}

template <>
std::vector<Collider*> Entity::getComponents()
{
    std::vector<Component*>& _colliders = components[typeid(Collider).hash_code()];
    std::vector<Collider*> colliders; colliders.resize(_colliders.size());

    for (unsigned i(0) ; i < _colliders.size() ; i++)
        colliders[i] = static_cast<Collider*>(_colliders[i]);

    return colliders;
}

template <>
std::vector<Script*> Entity::getComponents()
{
    std::vector<Component*>& _scripts = components[typeid(Script).hash_code()];
    std::vector<Script*> scripts; scripts.resize(_scripts.size());

    for (unsigned i(0) ; i < _scripts.size() ; i++)
        scripts[i] = static_cast<Script*>(_scripts[i]);

    return scripts;
}
