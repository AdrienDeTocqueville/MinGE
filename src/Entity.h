#ifndef ENTITY_H
#define ENTITY_H

#include "Utility/Tag.h"

#include <typeindex>

class Component;
class Transform;
class Collider;
class Script;

// TODO: use unique_ptr<Component>
typedef std::unordered_map<std::type_index, std::vector<Component*>> ComponentMap;
typedef std::unordered_map<std::type_index, std::size_t> SizeMap;


class Entity final
{
    public:
        /// Method (public)
            void destroy();

            template<typename T>
            bool has()
            {
                std::vector<Component*>* vec = nullptr;

                if (std::is_base_of<Collider, T>::value)
                {
                    for (Component* c: components[getColliderTypeIndex()])
                        if (typeid(*c) == typeid(T))
                            return true;

                    return false;
                }

                else
                    return !components[typeid(T)].empty();
            }

            template<typename T, typename... Args>
            Entity* insert(Args&&... args)
            {
                if (typeid(T) == typeid(Transform))
                {
                    if (tr)
                        Error::add(USER_ERROR, "Impossible to insert a Transform component");
                    else
                    {
                        T* c = new T(args...);
                        c->entity = this;

                        tr = reinterpret_cast<Transform*>(c);
                    }

                    return this;
                }

                else if (std::is_base_of<Collider, T>::value)
                    insertComponent(new T(args...), typeid(Collider));

                else
                {
                    if (std::is_base_of<Script, T>::value)
                        scriptSizes[typeid(T)] = sizeof(T);

                    insertComponent(new T(args...), typeid(T));
                }

                return this;
            }

            template<typename T>
            void remove()
            {
                if (std::is_base_of<Collider, T>::value)
                    removeComponent(typeid(T), getColliderTypeIndex());

                else
                    removeComponent(typeid(T), typeid(T));
            }

            template<typename T>
            void removeAll()
            {
                if (std::is_base_of<Collider, T>::value)
                    removeComponents(typeid(T), getColliderTypeIndex());

                else
                    removeComponents(typeid(T), typeid(T));
            }


        /// Getters (public)
            template<typename T>
            T* find()
            {
                if (std::is_base_of<Collider, T>::value)
                {
                    for (Component* c: components[getColliderTypeIndex()])
                        if (typeid(*c) == typeid(T))
                            return static_cast<T*>(c);

                    return nullptr;
                }

                else
                {
                    const auto& vec = components[typeid(T)];

                    return (vec.size() ? static_cast<T*>(vec.front()) : nullptr);

                }
            }

            template <typename T>
            std::vector<T*> findAll()
            {
                std::vector<Component*>* vec = nullptr;
                std::vector<T*> allComponents;

                if (std::is_base_of<Collider, T>::value)
                    vec = &components[getColliderTypeIndex()];

                else
                    vec = &components[typeid(T)];

                allComponents.reserve(vec->size());

                for (Component* c: *vec)
                    if (typeid(*c) == typeid(T))
                        allComponents.push_back( static_cast<T*>(c) );

                return allComponents;
            }

        /// Methods (static)
            static Entity* create(const Tag& _tag, bool _prototype = false, vec3 _position = vec3(0.0f), vec3 _rotation = vec3(0.0f), vec3 _scale = vec3(1.0f));
            static Entity* clone(Entity* _entity, vec3 _position, vec3 _rotation = vec3(0.0f), vec3 _scale = vec3(1.0f));
            static Entity* clone(Entity* _entity);
            static void clear();

            static Entity* findByTag(const Tag& _tag, bool _allowPrototypes = true);
            static std::list<Entity*> findAllByTag(const Tag& _tag, bool _allowPrototypes = true);

        /// Attributes (public)
            const bool prototype;

            Tag tag;

    private:
        /// Methods (private)
            Entity(Tag _tag, bool _prototype);
            ~Entity();

            void insertComponent(Component* _component, std::type_index _typeid);

            void removeComponent(std::type_index _componentTypeid, std::type_index _typeid);
            void removeComponents(std::type_index _componentTypeid, std::type_index _typeid);

            void getComponents();

        /// Getter (private)
            std::type_index getColliderTypeIndex();
            std::type_index getScriptTypeIndex();

        /// Attributes (private)
            ComponentMap components;

            Transform* tr;

        /// Attributes (static)
            static std::list<Entity*> entities;

            static SizeMap scriptSizes;
};

template <>
void Entity::remove<Transform>();

template <>
Transform* Entity::find<Transform>();


template <>
void Entity::removeAll<Transform>() = delete;

template <>
void Entity::removeAll<Collider>();


template <>
std::vector<Transform*> Entity::findAll() = delete;

template <>
std::vector<Component*> Entity::findAll();

template <>
std::vector<Collider*> Entity::findAll();


#endif // ENTITY_H
