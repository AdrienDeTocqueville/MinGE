#include "Physic/DistanceConstraint.h"

#include "Components/RigidBody.h"
#include "Components/Transform.h"

#include "Entity.h"

std::vector<DistanceConstraint*> DistanceConstraint::joints;

DistanceConstraint::DistanceConstraint(RigidBody* _a, vec3 _anchorPointA, RigidBody* _b, vec3 _anchorPointB, float _maxDistance):
	bodies{ _a, _b },
	anchorPointA(_anchorPointA), anchorPointB(_anchorPointB),
	maxDistance(_maxDistance), squaredMaxDistance(_maxDistance * _maxDistance)
{
	joints.push_back(this);
}

bool DistanceConstraint::positionConstraint()
{
	// Détermination du vecteur allant du CDG au point d’attache sur les solides A et B
	qA = bodies[0]->find<Transform>()->vectorToWorld(anchorPointA);
		pointA = bodies[0]->getCOM() + qA;

	qB = bodies[1]->find<Transform>()->vectorToWorld(anchorPointB);
		pointB = bodies[1]->getCOM() + qB;


	Debug::drawLine(pointA, pointB, vec3(0.8, 1, 0.6));

	// On calcule d'abord les distances au carré pour les performances
	float squaredDistance = length2(pointA - pointB);

	// Si C >= 0, on passe la contrainte
	if (squaredMaxDistance - squaredDistance >= 0.0f)
		return false;

	// On calcule n, le vecteur unitaire de B vers A
	n = vec3(0.0f);
	distance = sqrt(squaredDistance);

	if (distance > EPSILON)
		n = (pointA - pointB) / distance;

	return true;
}

void DistanceConstraint::velocityConstraint(float _dt)
{
	// Compute J
		vec3 J[4] = { -n, -cross(qA, n), n, cross(qB, n) };

	//					  1
	// Compute Meff =   ----------
	//				  J m^-1 J^t
		float linear0 = bodies[0]->iM * dot(J[0], J[0]);
		float angular0 = dot(bodies[0]->iI * J[1], J[1]);

		float linear1 = bodies[1]->iM * dot(J[2], J[2]);
		float angular1 = dot(bodies[1]->iI * J[3], J[3]);

		float Meff = 1.0f / (linear0 + linear1 + angular0 + angular1);

	// Compute lambda = -Meff * (Jv + b)
		float Jv = dot(J[0], bodies[0]->linearVelocity) + dot(J[1], bodies[0]->angularVelocity) +
				   dot(J[2], bodies[1]->linearVelocity) + dot(J[3], bodies[1]->angularVelocity);

		float b = (BETA / _dt) * (maxDistance - distance);

		float lambda = -Meff * (Jv + b);

	// Clamp impulse applied
		float oldLambda = accumulatedLambda;
		accumulatedLambda = max(accumulatedLambda + lambda, 0.0f);

		lambda = accumulatedLambda - oldLambda;

	// Compute and apply impulse = J^t * lambda
		bodies[0]->applyImpulse(J[0], J[1], lambda);
		bodies[1]->applyImpulse(J[2], J[3], lambda);
}

////			Joint to COM
//// easier to understand because there is no angular velocity
//
//// Compute J
//	vec3 J = normalize(point - anchorPoint);
//
////					  1
//// Compute m_eff =  ---------
////				  J m^-1 J^t
//	float Meff = 1.0f / (body->iM * dot(J, J));
//
//// Compute lambda = -m_eff * (Jv + b)
////		float b = (squaredDistance - squaredMaxDistance) * exp(-beta * )
//	float b = (BETA / _dt) * (length(point - anchorPoint) - maxDistance);
//	float lambda = -Meff * (dot(J, body->velocity) + b);
//
//// Clamp impulse applied
//	float oldImpulse = accumulatedLambda;
//	accumulatedLambda += lambda;
//	accumulatedLambda = min(accumulatedLambda, 0.0f);
//	float deltaLambda = accumulatedLambda - oldImpulse;
//
//// Compute impulse = J^t * lambda
//	vec3 impulse = deltaLambda * J;
//
//body->velocity += body->iM * impulse;

DistanceConstraint::~DistanceConstraint()
{ }
