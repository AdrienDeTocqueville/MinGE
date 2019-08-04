#ifndef COMPONENT_H
#define COMPONENT_H

#include "Utility/Debug.h"
#include "Entity.h"

class Component
{
    friend class Entity;

    public:
        Component();
        virtual ~Component();

        /// Methods (public)
            virtual Component* clone() const = 0;

        /// Getters
            Entity*    getEntity() const    { return entity; }

            template <typename T> bool has() const
            {
                return entity->has<T>();
            }

            template <typename T> void insert() const
            {
                entity->insert<T>();
            }

            template <typename T> void remove() const
            {
                entity->remove<T>();
            }

            template <typename T> T* find() const
            {
                return entity->find<T>();
            }

            template <typename T> std::vector<T*> findAll() const
            {
                return entity->findAll<T>();
            }

#ifdef DEBUG
            static int instances;
#endif

    protected:
        /// Methods (private)
            virtual void onRegister() {};
            virtual void onDeregister() {};

        /// Attributes (protected)
            Entity* entity;
            Transform* tr;
};

#endif // COMPONENT_H
