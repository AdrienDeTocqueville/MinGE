#pragma once

#include "Components/Component.h"

#include "Assets/Material.h"
#include "Assets/Mesh.h"

class Skybox : public Component
{
public:
	Skybox();
	Skybox(MaterialRef material);

	/// Methods (public)
	virtual Skybox* clone() const override;

	void render(struct RenderContext *ctx, uint32_t view_id) const;

private:
	MaterialRef sky;
	MeshRef mesh;
};
