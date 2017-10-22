#ifndef CONTACTCONSTRAINT_H
#define CONTACTCONSTRAINT_H

#include <functional>
#include <typeindex>

#include "Physic/Constraint.h"

#define BETA 0.3f

class Entity;
class Collider;
class RigidBody;

struct Manifold;

class ContactConstraint : public Constraint
{
    public:
        ContactConstraint(Collider* _a, Collider* _b);
        virtual ~ContactConstraint();

        bool positionConstraint();          // Generate contact information
        void velocityConstraint(float _dt); // Solve impulse and apply

        void sendData();                    // Callback for scripts

    /// Attributes (public)
        Entity* entities[2];
        RigidBody* bodies[2];
        Collider*  colliders[2];

        Manifold* manifold;

        unsigned type;  // 0: Collision
                        // 1: Trigger

        float re;              // Mixed restitution
        float df;              // Mixed dynamic friction
        float sf;              // Mixed static friction

        vec3 qA, qB;
        float velAlongNormal;
        float accumulatedFrictionU, accumulatedFrictionV;

};

struct Manifold
{
    vec3 points[2];

    float penetration;  // penetration of deepest point
    vec3 normal;        // Normal to the plane of collision
    vec3 u, v;          // (normal, u, v) is a basis of R^3. Used for friction
    vec3 t;
};

struct Collision
{
    Entity* entity;
    RigidBody* body;
    Collider*  collider;

    vec3 point;

    vec3 normal;
    vec3 impulse;
    vec3 relativeVelocity;
};

class Dispatcher
{
    public:
        typedef std::pair<std::type_index, std::type_index> key;
        typedef std::function<Manifold*(Collider*, Collider*)> collisionFunction;

        static void fill();
        static void clear();

        static Manifold* getManifold(Collider* _a, Collider* _b);

        template <typename A, typename B>
        static void addEntry(collisionFunction func)
        {
            std::type_index a = typeid(A);
            std::type_index b = typeid(B);

            if (a > b)
                std::swap(a, b);

            functions[ key(a, b) ] = func;
        }

        static std::map<key, collisionFunction> functions;
};

#endif // CONTACTCONSTRAINT_H
