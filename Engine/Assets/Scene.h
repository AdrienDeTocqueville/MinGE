#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Utility/Tag.h"

typedef std::shared_ptr<class Scene> SceneRef;

typedef std::shared_ptr<class Mesh> MeshRef;
typedef std::shared_ptr<class Material> MaterialRef;

class Scene
{
public:
	static SceneRef create(const std::string &file);

	void spawn();
	void despawn();

	class Entity *find_prototype(const Tag &_tag);
	class Entity *find_entity(const Tag &_tag);

	~Scene();

private:
	Scene(): spawned(false) {}
	bool load(const std::string &file);

	std::vector<class Entity*> nodes, roots;

	std::vector<MeshRef> meshes;
	std::vector<MaterialRef> materials;

	std::vector<class Entity*> entities;
	bool spawned;
};
