#pragma once

#include "Components/Component.h"
#include "Assets/Material.h"
#include "Assets/Mesh.h"

class Skybox : public Component
{
public:
	Skybox(vec3 c0 = vec3(0.67f, 0.92f, 1.0f), vec3 c1 = vec3(0.208608f, 0.348608f, 0.478608f));
	Skybox(MaterialRef material);

	/// Methods (public)
	virtual Skybox* clone() const override;

	void render(struct CommandBucket *bucket, uint32_t view_id) const;

private:
	MaterialRef sky;
	MeshRef cube;
};
