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
                    vec = &components[getColliderTypeIndex()];

                else if (std::is_base_of<Script, T>::value)
                    vec = &components[getScriptTypeIndex()];

                else
                    return !components[typeid(T)].empty();


                for (Component* c: *vec)
                    if (typeid(*c) == typeid(T))
                        return true;

                return false;
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

                else if (std::is_base_of<Script, T>::value)
                {
                    scriptSizes[typeid(T)] = sizeof(T);

                    insertComponent(new T(args...), typeid(Script));
                }

                else
                    insertComponent(new T(args...), typeid(T));

                return this;
            }

            template<typename T>
            void remove()
            {
                if (std::is_base_of<Collider, T>::value)
                    removeComponent(typeid(T), getColliderTypeIndex());

                else if (std::is_base_of<Script, T>::value)
                    removeComponent(typeid(T), getScriptTypeIndex());

                else
                    removeComponent(typeid(T), typeid(T));
            }

            template<typename T>
            void removeAll()
            {
                if (std::is_base_of<Collider, T>::value)
                    removeComponents(typeid(T), getColliderTypeIndex());

                else if (std::is_base_of<Script, T>::value)
                    removeComponents(typeid(T), getScriptTypeIndex());

                else
                    removeComponents(typeid(T), typeid(T));
            }


        /// Getters (public)
            Tag getTag() const;

            template<typename T>
            T* find()
            {
                std::vector<Component*>* vec = nullptr;

                if (std::is_base_of<Collider, T>::value)
                    vec = &components[getColliderTypeIndex()];

                else if (std::is_base_of<Script, T>::value)
                    vec = &components[getScriptTypeIndex()];

                else
                {
                    vec = &components[typeid(T)];
                    return (vec->size() ? static_cast<T*>(vec->front()) : nullptr);

                }

                for (Component* c: *vec)
                    if (typeid(*c) == typeid(T))
                        return static_cast<T*>(c);

                return nullptr;
            }

            template <typename T>
            std::vector<T*> findAll()
            {
                std::vector<Component*>* vec = nullptr;
                std::vector<T*> allComponents;

                if (std::is_base_of<Collider, T>::value)
                    vec = &components[getColliderTypeIndex()];

                else if (std::is_base_of<Script, T>::value)
                    vec = &components[getScriptTypeIndex()];

                else
                    vec = &components[typeid(T)];

                allComponents.reserve(vec->size());

                for (Component* c: *vec)
                    if (typeid(*c) == typeid(T))
                        allComponents.push_back( static_cast<T*>(c) );

                return allComponents;
            }

        /// Methods (static)
            static Entity* create(std::string _tag, bool _prototype = false, vec3 _position = vec3(0.0f), vec3 _rotation = vec3(0.0f), vec3 _scale = vec3(1.0f));
            static Entity* create(Tag _tag, bool _prototype = false, vec3 _position = vec3(0.0f), vec3 _rotation = vec3(0.0f), vec3 _scale = vec3(1.0f));
            static Entity* clone(Entity* _entity, vec3 _position = vec3(0.0f), vec3 _rotation = vec3(0.0f), vec3 _scale = vec3(1.0f));
            static void clear();

            static Entity* findByTag(std::string _tag);
            static std::list<Entity*> findAllByTag(std::string _tag);

            static Entity* findByTag(Tag _tag);
            static std::list<Entity*> findAllByTag(Tag _tag);

        /// Attributes (public)
            const bool prototype;

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
            Tag tag;

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
void Entity::removeAll<Script>();


template <>
std::vector<Transform*> Entity::findAll() = delete;

template <>
std::vector<Component*> Entity::findAll();

template <>
std::vector<Collider*> Entity::findAll();

template <>
std::vector<Script*> Entity::findAll();

#endif // ENTITY_H
