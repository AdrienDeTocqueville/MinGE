#include "PhysicMaterial.h"

std::unordered_map<std::string, PhysicMaterial*> PhysicMaterial::materials;
PhysicMaterial* PhysicMaterial::base = nullptr;

PhysicMaterial::PhysicMaterial(std::string _name, float _restitution, float _dF, float _sF):
    name(_name),
    restitution(_restitution),
    dynamicFriction(_dF), staticFriction(_sF)
{
    materials[name] = this;
}

/// Methods (static)
PhysicMaterial* PhysicMaterial::get(std::string _name)
{
    auto it = materials.find(_name);

    if (it == materials.end())
    {
        Error::add(MINGE_ERROR, "PhysicMaterial::get() -> unknown material: " + _name);
        return nullptr;
    }
    else
        return it->second;
}

void PhysicMaterial::clear()
{
    for(auto it: materials)
        delete it.second;

    materials.clear();
}

/// Setter
void PhysicMaterial::setRestitution(float _restitution)
{
    restitution = _restitution;
}
