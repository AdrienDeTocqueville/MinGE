#include "MinGE.h"

std::list<Entity*> Entity::entities;
SizeMap Entity::scriptSizes;

Entity::Entity(Tag _tag, bool _prototype):
    prototype(_prototype), tag(_tag),
    tr(nullptr)
{
    entities.push_back(this);
}

Entity::~Entity()
{
    // free components
    for (auto& componentPair: components)
    {
        auto csize = componentPair.second.size();
        for (unsigned i(0) ; i < csize ; i++)
        {
            if (!prototype)
                componentPair.second.back()->onDeregister();

            delete componentPair.second.back();
            componentPair.second.pop_back();
        }
    }

    delete tr;
}

/// Methods (public)
void Entity::destroy()
{
    entities.remove(this);

    delete this;
}

/// Getters
Tag Entity::getTag() const
{
    return tag;
}

/// Methods (static)
Entity* Entity::create(std::string _tag, bool _prototype, vec3 _position, vec3 _rotation, vec3 _scale)
{
    return Entity::create(Tag(_tag), _prototype, _position, _rotation, _scale);
}

Entity* Entity::create(Tag _tag, bool _prototype, vec3 _position, vec3 _rotation, vec3 _scale)
{
    return (new Entity(_tag, _prototype))->add<Transform>(_position, _rotation, _scale);
}

Entity* Entity::clone(Entity* _entity, vec3 _position, vec3 _rotation, vec3 _scale)
{
    Entity* e = Entity::create(_entity->tag, false, _position, _rotation, _scale);

    for (auto& componentPair: _entity->components)
    {
        auto& components = e->components[componentPair.first];

        if (componentPair.first == typeid(Script))
        {
            for (Component* component: componentPair.second)
            {
                size_t s = scriptSizes[typeid(*component)];

                Component* c = reinterpret_cast<Component*>(operator new(s));
                memcpy(c, component, s);
                Component::instances++;

                components.push_back(c);

                c->entity = e;
                c->tr = e->tr;
                c->onRegister();
            }
        }

        else
        {
            for (Component* component: componentPair.second)
            {
                Component* c = component->clone();
                components.push_back(c);

                c->entity = e;
                c->tr = e->tr;
                c->onRegister();
            }
        }
    }

    return e;
}

void Entity::clear()
{
    for(Entity* entity: entities)
        delete entity;

    entities.clear();
}

Entity* Entity::findByTag(std::string _tag)
{
    return findByTag(Tag(_tag));
}

std::list<Entity*> Entity::findAllByTag(std::string _tag)
{
    Tag tagId = Tag(_tag);

    std::list<Entity*> entities;

    for (Entity* entity: entities)
    {
        if (entity->tag == tagId)
            entities.push_back(entity);
    }

    return entities;
}

Entity* Entity::findByTag(Tag _tag)
{
    for (Entity* entity: entities)
    {
        if (entity->tag == _tag)
            return entity;
    }

    return nullptr;
}

std::list<Entity*> Entity::findAllByTag(Tag _tag)
{
    std::list<Entity*> entities;

    for (Entity* entity: entities)
    {
        if (entity->tag == _tag)
            entities.push_back(entity);
    }

    return entities;
}

/// Methods (private)
void Entity::addComponent(Component* _component, std::type_index _typeid)
{
    components[_typeid].push_back(_component);

    _component->entity = this;

    if (!prototype)
    {
        _component->tr = tr;
        _component->onRegister();
    }
}

void Entity::removeComponent(std::type_index _componentTypeid, std::type_index _typeid)
{
    std::vector<Component*>& vec = components[_typeid];

    for (unsigned i(0) ; i < vec.size() ; i++)
    {
        Component* c = vec[i];

        if (std::type_index(typeid(*c)) == _componentTypeid)
        {
            vec.erase(vec.begin() + i);

            if (!prototype)
                c->onDeregister();

            delete c;
            return;
        }
    }
}

void Entity::removeComponents(std::type_index _componentTypeid, std::type_index _typeid)
{
    std::vector<Component*>& vec = components[_typeid];

    for (unsigned i(0) ; i < vec.size() ; i++)
    {
        Component* c = vec[i];

        if (std::type_index(typeid(*c)) == _componentTypeid)
        {
            vec.erase(vec.begin() + i--);

            if (!prototype)
                c->onDeregister();

            delete c;
        }
    }
}

/// Getter (private)
std::type_index Entity::getColliderTypeIndex()
{
    return typeid(Collider);
}

std::type_index Entity::getScriptTypeIndex()
{
    return typeid(Script);
}

/// Other
//template<typename... Args>
//Entity* Entity::add<Transform>(Args&&... args)
//{
//    if (tr != nullptr)
//        Error::add(USER_ERROR, "Impossible to add a Transform component");
//    else
//        tr = new Transform(args...);
//
//    return this;
//}

template <>
void Entity::remove<Transform>()
{
    Error::add(USER_ERROR, "Impossible to remove the Transform component");
}

template <>
Transform* Entity::get()
{
    return tr;
}

template <>
void Entity::removeAll<Collider>()
{
    for (Component* collider: components[typeid(Collider)])
    {
        Collider* c = static_cast<Collider*>(collider);

        c->entity = nullptr;
        c->rigidBody = nullptr;

        c->onDeregister();
    }

    components[typeid(Collider)].clear();

    RigidBody* rb = get<RigidBody>();
    if (rb != nullptr)
        rb->computeMass();
}

template <>
void Entity::removeAll<Script>()
{
    for (Component* script: components[typeid(Script)])
    {
        Script* s = static_cast<Script*>(script);

        s->entity = nullptr;

        s->onDeregister();
    }

    components[typeid(Script)].clear();
}

template <>
std::vector<Component*> Entity::getAll()
{
    std::vector<Component*> _components;

    for (auto& componentPair: components)
        for (Component* component: componentPair.second)
            _components.push_back(component);

    return _components;
}

template <>
std::vector<Collider*> Entity::getAll()
{
    std::vector<Component*>& _colliders = components[typeid(Collider)];
    std::vector<Collider*> colliders; colliders.resize(_colliders.size());

    for (unsigned i(0) ; i < _colliders.size() ; i++)
        colliders[i] = static_cast<Collider*>(_colliders[i]);

    return colliders;
}

template <>
std::vector<Script*> Entity::getAll()
{
    std::vector<Component*>& _scripts = components[typeid(Script)];
    std::vector<Script*> scripts; scripts.resize(_scripts.size());

    for (unsigned i(0) ; i < _scripts.size() ; i++)
        scripts[i] = static_cast<Script*>(_scripts[i]);

    return scripts;
}
