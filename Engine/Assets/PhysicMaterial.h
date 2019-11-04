#pragma once

#include <memory>
#include "Utility/helpers.h"

typedef std::shared_ptr<class PhysicMaterial> PhysicMaterialRef;

class PhysicMaterial
{
	friend class Collider;

public:
	/// Methods (static)
	static PhysicMaterialRef create(float _restitution = 0.2f, float _dF = 0.6f, float _sF = 0.6f);
	static PhysicMaterialRef getDefault();

	/// Setter
	void setRestitution(float _restitution);

protected:
	PhysicMaterial(float _restitution, float _dF, float _sF);

	float restitution;
	float dynamicFriction, staticFriction;

	/// Attributes (static)
	static std::weak_ptr<PhysicMaterial> basic;
};
