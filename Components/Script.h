#ifndef SCRIPT_H
#define SCRIPT_H

#include "Components/Component.h"

struct Collision;

class Script : public Component
{
    friend class Entity;

    public:
        Script();
        virtual ~Script();

        /// Methods (public)
            virtual Script* clone() const override final;

            virtual void start      ();
            virtual void update     ();
            virtual void lateUpdate ();

            virtual void onCollision(const Collision& _collision);
            virtual void onTrigger  (Collider* _collider);

    private:
        /// Methods (private)
            virtual void onRegister() override;
            virtual void onDeregister() override;
};

#endif // SCRIPT_H
