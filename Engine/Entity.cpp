#include "MinGE.h"
#include <cstring>

std::vector<Entity*> Entity::entities;
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
	auto it = std::find(entities.begin(), entities.end(), this);
	if (it != entities.end())
	{
		*it = entities.back();
		entities.pop_back();
	}

	delete this;
}

/// Methods (static)
Entity* Entity::create(const Tag& _tag, bool _prototype, vec3 _position, vec3 _rotation, vec3 _scale)
{
	return (new Entity(_tag, _prototype))->insert<Transform>(_position, _rotation, _scale);
}

Entity* Entity::create(const Tag& _tag, bool _prototype, vec3 _position, quat _rotation, vec3 _scale)
{
	return (new Entity(_tag, _prototype))->insert<Transform>(_position, _rotation, _scale);
}

Entity* Entity::clone(Entity* _entity, vec3 _position, vec3 _rotation, vec3 _scale)
{
	Entity* e = Entity::create(_entity->tag, false, _position, _rotation, _scale);

	for (auto& componentPair: _entity->components)
	{
		auto& components = e->components[componentPair.first];

		for (Component* component: componentPair.second)
		{
			Component* c = component->clone();

			if (c == nullptr)
			{
				size_t s = scriptSizes[typeid(*component)];

				c = reinterpret_cast<Component*>(operator new(s));
				memcpy((void*)c, component, s);
#ifdef DEBUG
				Component::instances++;
#endif
			}

			components.push_back(c);

			c->entity = e;
			c->tr = e->tr;
		}
	}

	for (auto *child : _entity->find<Transform>()->getChildren())
	{
		Entity *c = child->entity;
		Entity::clone(c)->tr->setParent(e->tr);
	}

	for (auto& componentPair: e->components)
	{
		for (Component* component: componentPair.second)
			component->onRegister();
	}

	return e;
}

Entity* Entity::clone(Entity* _entity)
{
	Transform* tr = _entity->find<Transform>();

	return Entity::clone(_entity, tr->position, eulerAngles(tr->rotation), tr->scale);
}

void Entity::clear()
{
	for(Entity* entity: entities)
		delete entity;

	entities.clear();
}

Entity* Entity::findByTag(const Tag& _tag, bool _allowPrototypes)
{
	for (Entity* entity: entities)
	{
		if (entity->tag == _tag)
		{
			if (!entity->prototype || _allowPrototypes)
				return entity;
		}
	}

	return nullptr;
}

std::vector<Entity*> Entity::findAllByTag(const Tag& _tag, bool _allowPrototypes)
{
	std::vector<Entity*> samedTag;

	for (Entity* entity: entities)
	{
		if (entity->tag == _tag)
		{
			if (!entity->prototype || _allowPrototypes)
				samedTag.push_back(entity);
		}
	}

	return samedTag;
}

/// Methods (private)
void Entity::insertComponent(Component* _component, std::type_index _typeid)
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
//Entity* Entity::insert<Transform>(Args&&... args)
//{
//	if (tr != nullptr)
//		Error::add(Error::USER, "Impossible to add a Transform component");
//	else
//		tr = new Transform(args...);
//
//	return this;
//}

template <>
void Entity::removeAll<Collider>()
{
	for (Component* collider: components[typeid(Collider)])
	{
		Collider* c = static_cast<Collider*>(collider);

		if (!prototype)
			c->onDeregister();

		delete c;
	}

	components[typeid(Collider)].clear();

	RigidBody* rb = find<RigidBody>();
	if (rb != nullptr)
		rb->computeMass();
}

template <>
std::vector<Component*> Entity::findAll()
{
	std::vector<Component*> _components;

	for (auto& componentPair: components)
		for (Component* component: componentPair.second)
			_components.push_back(component);

	return _components;
}

template <>
std::vector<Collider*> Entity::findAll()
{
	std::vector<Component*>& _colliders = components[typeid(Collider)];
	std::vector<Collider*> colliders; colliders.resize(_colliders.size());

	for (unsigned i(0) ; i < _colliders.size() ; i++)
		colliders[i] = static_cast<Collider*>(_colliders[i]);

	return colliders;
}

