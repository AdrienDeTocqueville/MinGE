#ifndef COMPONENT_H
#define COMPONENT_H

#include "Utility/Debug.h"
#include "Entity.h"

class Component
{
    friend class Entity;

    public:
        Component();

        /// Methods (public)
            virtual Component* clone() const = 0;

        /// Getters
            Entity*    getEntity() const    { return entity; }

            template <typename T> T* get() const
            {
                return entity->get<T>();
            }

            template <typename T> std::vector<T*> getAll() const
            {
                return entity->getAll<T>();
            }

            static float deltaTime;
            static int instances;

    protected:
        /// Methods (private)
            virtual ~Component();

            virtual void onRegister();
            virtual void onDeregister();

        /// Attributes (protected)
            Entity* entity;
            Transform* tr;
};

#endif // COMPONENT_H
