#ifndef DISTANCECONSTRAINT_H
#define DISTANCECONSTRAINT_H

#include "Physic/Constraint.h"

class RigidBody;

class DistanceConstraint : public Constraint
{
	public:
		DistanceConstraint(RigidBody* _a, vec3 _anchorPointA, RigidBody* _b, vec3 _anchorPointB, float _maxDistance);

		/// Methods (public)
			bool positionConstraint();
			void velocityConstraint(float _dt);

		/// Methods (static)
			static void clear()
			{
				for (unsigned i(0) ; i < joints.size() ; i++)
					delete joints[i];

				joints.clear();
			}

	private:
		virtual ~DistanceConstraint();

		RigidBody* bodies[2];

		vec3 pointA, pointB, qA, qB;
		vec3 n;

		vec3 anchorPointA, anchorPointB;

		float maxDistance, squaredMaxDistance;
		float distance;

		static std::vector<DistanceConstraint*> joints;
};

#endif // DISTANCECONSTRAINT_H
