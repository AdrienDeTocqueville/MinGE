#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "Utility/helpers.h"

#define BETA 0.3f

class Constraint
{
	public:
		Constraint();
		virtual ~Constraint();

		virtual bool positionConstraint() = 0;
		virtual void velocityConstraint(float _dt) = 0;

	protected:
		float accumulatedLambda;
};

#endif // CONSTRAINT_H
