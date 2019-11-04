#include "PhysicMaterial.h"
#include "Utility/Error.h"

std::weak_ptr<PhysicMaterial> PhysicMaterial::basic;

PhysicMaterial::PhysicMaterial(float _restitution, float _dF, float _sF):
	restitution(_restitution),
	dynamicFriction(_dF), staticFriction(_sF)
{ }

/// Method (static)
PhysicMaterialRef PhysicMaterial::create(float _restitution, float _dF, float _sF)
{
	return PhysicMaterialRef(new PhysicMaterial(_restitution, _dF, _sF));
}

PhysicMaterialRef PhysicMaterial::getDefault()
{
	if (auto shared = basic.lock())
		return shared;

	auto shared = PhysicMaterial::create();

	basic = shared;
	return shared;
}

/// Setter
void PhysicMaterial::setRestitution(float _restitution)
{
	restitution = _restitution;
}
