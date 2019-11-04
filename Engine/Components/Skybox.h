#pragma once

#include "Components/Component.h"
#include "Assets/Mesh.h"

class Skybox : public Component
{
public:
	Skybox();

	/// Methods (public)
	virtual Skybox* clone() const override;

	void render() const;

private:
	MeshRef sky;
};
