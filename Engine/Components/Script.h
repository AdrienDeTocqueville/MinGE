#ifndef SCRIPT_H
#define SCRIPT_H

#include "Components/Component.h"

struct Collision;

class Script : public Component
{
    public:
        Script();
        virtual ~Script();

        /// Methods (public)
            virtual Component* clone() override final;

            virtual void attach(Entity* _entity) override final;
            virtual void registerComponent() override final;
            virtual void deregisterComponent() override final;

            virtual void start      () {}
            virtual void update     () {}
            virtual void lateUpdate () {}

            virtual void onCollision(const Collision& _collision) {}
            virtual void onTrigger  (Collider* _collider) {}
};

#endif // SCRIPT_H
