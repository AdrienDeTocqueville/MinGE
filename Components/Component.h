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

            virtual void attach(Entity* _entity);
            virtual void detach();
            virtual void registerComponent() { registered = true; }
            virtual void deregisterComponent() { registered = false; }

            void destroy();

        /// Getters
            Entity*    getEntity() const    { return entity; }
            Transform* getTransform() const { return tr;     }

            template <typename T>
            T* getComponent() const         { return entity->getComponent<T>(); }
            template <typename T>
            std::vector<T*> getComponents() const         { return entity->getComponents<T>(); }

            static float deltaTime;
            static unsigned instances;

    protected:
        virtual ~Component();

        /// Attributes (protected)
            Entity* entity;
            Transform* tr;

            bool registered;
};

#endif // COMPONENT_H
