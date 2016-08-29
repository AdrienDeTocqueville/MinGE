#ifndef ENTITY_H
#define ENTITY_H

#include "includes.h"

#include <typeinfo>

class Component;
class Transform;
class Collider;
class Script;

typedef std::map<size_t, std::vector<Component*>> ComponentMap;


class Entity final
{
    friend class Component;

    friend class Transform;
    friend class Collider;
    friend class Script;

    public:
        Entity(std::string _tag);
        Entity(std::string _tag, std::vector<Component*> _components, bool _prototype = false);

        /// Method (public)
            Entity* clone(vec3 _position = vec3(0.0f), vec3 _rotation = vec3(0.0f), vec3 _scale = vec3(1.0f));

            void addComponent(Component* _component);

            template<typename T>
            void removeComponent()
            {
                auto& vec = components[typeid(T).hash_code()];

                if (vec.size() != 1)
                    return;

                removeComponent(vec[0]);
            }

            void removeComponent(Component* _component);


            template<typename T>
            void removeComponents()
            {
                for (Component* component: components[typeid(T).hash_code()])
                    removeComponent(component);
            }

            void destroy();

        /// Methods (static)
            static unsigned getTagId(std::string _tag);
            static Entity* findByTag(std::string _tag);
            static std::list<Entity*> findAllByTag(std::string _tag);

            static void clear();

        /// Attributes (static)
            static std::vector< std::string > tags;
            static std::list<Entity*> entities;

        /// Getters
            unsigned getTag() const;
            std::string getTagName() const;

            size_t getColliderHashCode();
            size_t getScriptHashCode();

            Transform* getTransform() const;

            template <typename T>
            T* getComponent()
            {
                std::vector<Component*>* vec = nullptr;

                if (std::is_base_of<Collider, T>::value)
                    vec = &components[getColliderHashCode()];

                else if (std::is_base_of<Script, T>::value)
                    vec = &components[getScriptHashCode()];

                else
                    vec = &components[typeid(T).hash_code()];

                if (vec->size() != 0)
                {
                    for (unsigned i(0) ; i < vec->size() ; i++)
                    {
                        if (typeid(*(*vec)[i]) == typeid(T))
                            return static_cast<T*>((*vec)[i]);
                    }
                }

                return nullptr;
            }

            template <typename T>
            std::vector<T*> getComponents()
            {
                std::vector<Component*>* vec = nullptr;

                if (std::is_base_of<Collider, T>::value)
                    vec = &components[getColliderHashCode()];

                else if (std::is_base_of<Script, T>::value)
                    vec = &components[getScriptHashCode()];

                else
                    vec = &components[typeid(T).hash_code()];

                std::vector<T*> components; components.resize(vec->size());

                for (unsigned i(0) ; i < vec->size() ; i++)
                    components[i] = static_cast<T*>((*vec)[i]);

                return components;
            }

    private:
        ~Entity();

        /// Attributes (private)
            unsigned tag;
            bool prototype;

            ComponentMap components;

            Transform* tr;
};

template <>
void Entity::removeComponents<Collider>();

template <>
void Entity::removeComponents<Script>();

template <>
std::vector<Component*> Entity::getComponents();

template <>
std::vector<Collider*> Entity::getComponents();

template <>
std::vector<Script*> Entity::getComponents();

#endif // ENTITY_H
