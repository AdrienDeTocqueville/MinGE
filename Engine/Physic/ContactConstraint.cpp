#include "Physic/ContactConstraint.h"
#include "Physic/CollisionDetection.h"

#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/Collider.h"
#include "Components/Script.h"
#include "Entity.h"

ContactConstraint::ContactConstraint(Collider* _a, Collider* _b):
    entities{_a->getEntity(), _b->getEntity()},
    bodies{ _a->rigidBody, _b->rigidBody},
    colliders{ _a, _b },
    manifold(nullptr), type(0),
    velAlongNormal(0.0f),
    accumulatedFrictionU(0.0f), accumulatedFrictionV(0.0f)
{ }

ContactConstraint::~ContactConstraint()
{ }

bool ContactConstraint::positionConstraint()
{
    manifold = Dispatcher::getManifold(colliders[0], colliders[1]);

    if (manifold == nullptr || manifold->penetration >= 0.0f)
        return false;


    if (colliders[0]->getTrigger() || colliders[1]->getTrigger())
    {
        type = 1;
        return true;
    }

    // Calculate average restitution
    re = colliders[0]->getRestitution() + colliders[1]->getRestitution();   re *= 0.5f;

    // Calculate dynamic and static friction
    df = std::sqrt( colliders[0]->getDynamicFriction() * colliders[1]->getDynamicFriction() );
    sf = std::sqrt( colliders[0]->getStaticFriction()  * colliders[1]->getStaticFriction()  );


    // Vector from COM to contact points
        qA = manifold->points[0] - bodies[0]->getCOM();
        qB = manifold->points[1] - bodies[1]->getCOM();

    for (unsigned i(0) ; i < 3 ; i++)
    {
        qA[i] = ((int)(qA[i] * 100.0f)) * 0.01f;
        qB[i] = ((int)(qB[i] * 100.0f)) * 0.01f;
    }

    vec3 bCb = cross( bodies[1]->angularVelocity, qB );
    vec3 aCa = cross( bodies[0]->angularVelocity, qA );

    for (unsigned i(0) ; i < 3 ; i++)
    {
        if (isnan(bCb[i]))
        {
            std::cout << "naaaaaaaaaaaaaaan" << std::endl;
            bCb[i] = 0.0f;
        }
    }
    for (unsigned i(0) ; i < 3 ; i++)
    {
        if (isnan(aCa[i]))
        {
            std::cout << "nooooooooooooon" << std::endl;
            aCa[i] = 0.0f;
        }
    }

    vec3 relativeVelocity = bodies[1]->linearVelocity + bCb -
                            bodies[0]->linearVelocity - aCa;
    velAlongNormal = dot( relativeVelocity, manifold->normal );

    return true;
}

void ContactConstraint::velocityConstraint(float _dt)
{
    /// Contact impulse
    // Compute J
        vec3 J[4] = { -manifold->normal, -cross(qA, manifold->normal), manifold->normal, cross(qB, manifold->normal) };

    //                      1
    // Compute Meff =   ----------
    //                  J m^-1 J^t
        float linear0 = bodies[0]->iM * dot(J[0], J[0]);
        float angular0 = dot(bodies[0]->iI * J[1], J[1]);

        float linear1 = bodies[1]->iM * dot(J[2], J[2]);
        float angular1 = dot(bodies[1]->iI * J[3], J[3]);

        float Meff = 1.0f / (linear0 + linear1 + angular0 + angular1);

    // Compute lambda = -Meff * (Jv + b)
        float Jv = dot(J[0], bodies[0]->linearVelocity) + dot(J[1], bodies[0]->angularVelocity) +
                   dot(J[2], bodies[1]->linearVelocity) + dot(J[3], bodies[1]->angularVelocity);
        float b = re * velAlongNormal + (BETA / _dt) * min(manifold->penetration + EPSILON, 0.0f);

        float lambda = -Meff * (Jv + b);

    // Clamp impulse applied
        float oldLambda = accumulatedLambda;
        accumulatedLambda = max(accumulatedLambda + lambda, 0.0f);

        lambda = accumulatedLambda - oldLambda;

    // Compute and apply impulse = J^t * lambda
        bodies[0]->applyImpulse(J[0], J[1], lambda);
        bodies[1]->applyImpulse(J[2], J[3], lambda);


    /// Friction impulse
        // U axis
        vec3 J1[4] = { -manifold->u, -cross(qA, manifold->u), manifold->u, cross(qB, manifold->u) };

        angular0 = dot(bodies[0]->iI * J1[1], J1[1]);
        angular1 = dot(bodies[1]->iI * J1[3], J1[3]);
        Meff = 1.0f / (bodies[0]->iM + bodies[1]->iM + angular0 + angular1);

        float J1v = dot(J1[0], bodies[0]->linearVelocity) + dot(J1[1], bodies[0]->angularVelocity) +
                    dot(J1[2], bodies[1]->linearVelocity) + dot(J1[3], bodies[1]->angularVelocity);

        float lambdaU = -Meff * J1v;

        // V axis
        vec3 J2[4] = { -manifold->v, -cross(qA, manifold->v), manifold->v, cross(qB, manifold->v) };

        angular0 = dot(bodies[0]->iI * J2[1], J2[1]);
        angular1 = dot(bodies[1]->iI * J2[3], J2[3]);
        Meff = 1.0f / (bodies[0]->iM + bodies[1]->iM + angular0 + angular1);

        float J2v = dot(J2[0], bodies[0]->linearVelocity) + dot(J2[1], bodies[0]->angularVelocity) +
                    dot(J2[2], bodies[1]->linearVelocity) + dot(J2[3], bodies[1]->angularVelocity);

        float lambdaV = -Meff * J2v;

    float oldLambdaU = accumulatedFrictionU;
    float oldLambdaV = accumulatedFrictionV;

    accumulatedFrictionU = clamp(accumulatedFrictionU + lambdaU, -sf*oldLambda, sf*oldLambda);
    accumulatedFrictionV = clamp(accumulatedFrictionV + lambdaV, -sf*oldLambda, sf*oldLambda);

    lambdaU = accumulatedFrictionU - oldLambdaU;
    lambdaV = accumulatedFrictionV - oldLambdaV;

    bodies[0]->applyImpulse(J1[0], J1[1], lambdaU);
    bodies[0]->applyImpulse(J2[0], J2[1], lambdaV);

    bodies[1]->applyImpulse(J1[2], J1[3], lambdaU);
    bodies[1]->applyImpulse(J2[2], J2[3], lambdaV);
}

void ContactConstraint::sendData()
{
    if (type == 0)
    {
        qA = manifold->points[0] - bodies[0]->getCOM();
        qB = manifold->points[1] - bodies[1]->getCOM();


        Collision col;

        col.normal = manifold->normal;
        col.impulse = accumulatedLambda * manifold->normal;
        col.relativeVelocity = bodies[1]->linearVelocity + cross( bodies[1]->angularVelocity, qB ) -
                               bodies[0]->linearVelocity - cross( bodies[0]->angularVelocity, qA );

        for (unsigned i(0) ; i < 2 ; i++)
        {
            col.body = bodies[1-i];
            col.entity = entities[1-i];
            col.collider = colliders[1-i];

            col.point = manifold->points[1-i];

            for (auto script: colliders[i]->getEntity()->getComponents<Script>())
                script->onCollision(col);
        }
    }
    else
    {
        for (unsigned i(0) ; i < 2 ; i++)
            for (auto script: colliders[i]->getEntity()->getComponents<Script>())
                script->onTrigger(colliders[1-i]);
    }
}

#include "Components/Sphere.h"

/// Dispatcher
std::map<Dispatcher::key, Dispatcher::collisionFunction> Dispatcher::functions;

void Dispatcher::fill()
{
    addEntry<Sphere, Sphere>(SphereSphere);
}

void Dispatcher::clear()
{
    functions.clear();
}

Manifold* Dispatcher::getManifold(Collider* _a, Collider* _b)
{
    std::size_t a = typeid(*_a).hash_code();
    std::size_t b = typeid(*_a).hash_code();

    if (a > b)
        std::swap(a, b);

    std::map<key, collisionFunction>::iterator it = functions.find( key(a, b) );

    if (it != functions.end())
        return (it->second)(_a, _b);

    else
    {
        Simplex simplex;
        if (!GJK(_a, _b, simplex))
            return nullptr;

        return EPA(_a, _b, simplex);
    }
}
