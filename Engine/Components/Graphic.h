#pragma once

#include "Components/Component.h"
#include "Assets/Mesh.h"

class Graphic : public Component
{
public:
	Graphic(Mesh* _mesh);
	Graphic(Mesh* _mesh, std::vector<MaterialRef> _materials);
	virtual ~Graphic();

	/// Methods (public)
	virtual Graphic* clone() const override;

	void render();

	/// Setters
	void setMesh(Mesh* _mesh);
	void setMesh(Mesh* _mesh, std::vector<MaterialRef> _materials);

	/// Getters
	Mesh* getMesh() const;
	AABB getAABB() const;

	const std::vector<MaterialRef>& getMaterials() const;

private:
	/// Methods (private)
	virtual void onRegister() override;
	virtual void onDeregister() override;

	/// Attributes
	Mesh* mesh;
	std::vector<MaterialRef> materials;
};
