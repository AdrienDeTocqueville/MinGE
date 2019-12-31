#pragma once

#include "Components/Component.h"

#include "Assets/Mesh.h"
#include "Assets/Material.h"

#include <initializer_list>

class Graphic : public Component
{
public:
	Graphic(MeshRef _mesh);
	Graphic(MeshRef _mesh, std::vector<MaterialRef> _materials);
	Graphic(MeshRef _mesh, std::initializer_list<MaterialRef> _materials);
	virtual ~Graphic();

	/// Methods (public)
	virtual Graphic* clone() const override;

	void render(struct RenderContext *ctx, uint32_t num_views, struct View const *views) const;

	/// Setters
	void setMesh(MeshRef _mesh);
	void setMesh(MeshRef _mesh, std::vector<MaterialRef> _materials);
	void setMesh(MeshRef _mesh, std::initializer_list<MaterialRef> _materials);

	/// Getters
	MeshRef getMesh() const;
	AABB getAABB() const;

	std::vector<MaterialRef>& getMaterials() { return materials; }
	const std::vector<MaterialRef>& getMaterials() const { return materials; }

private:
	/// Methods (private)
	void updateMesh(MeshRef _mesh);

	virtual void onRegister() override;
	virtual void onDeregister() override;

	/// Attributes
	MeshRef mesh;
	std::vector<MaterialRef> materials;
};
