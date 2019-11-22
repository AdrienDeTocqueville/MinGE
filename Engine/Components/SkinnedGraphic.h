#pragma once

#include "Components/Component.h"

#include "Assets/Mesh.h"
#include "Assets/Material.h"
#include "Assets/Animation.h"

#include <initializer_list>

struct Skeleton
{
	std::unordered_map<std::string, unsigned> bone_index;
	std::vector<mat4> offsets;
	std::vector<Transform*> nodes;
};

class SkinnedGraphic : public Component
{
public:
	SkinnedGraphic(Skeleton _skeleton, MeshRef _mesh, std::vector<MaterialRef> _materials, std::vector<AnimationRef> _animations);
	virtual ~SkinnedGraphic();

	/// Methods (public)
	virtual SkinnedGraphic* clone() const override;

	void animate();
	void render(struct CommandBucket *bucket) const;

	/// Getters
	MeshRef getMesh() const;
	AABB getAABB() const;

	const std::vector<MaterialRef>& getMaterials() const;
	const std::vector<AnimationRef>& getAnimations() const;

private:
	/// Methods (private)
	void updateMesh(MeshRef _mesh);

	virtual void onRegister() override;
	virtual void onDeregister() override;

	/// Attributes
	Skeleton skeleton;

	MeshRef mesh;
	std::vector<MaterialRef> materials;
	std::vector<AnimationRef> animations;
};
