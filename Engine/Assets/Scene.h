#pragma once

#include <string>
#include <vector>
#include <memory>

struct Scene
{
	Scene(const std::string &file);

	void instantiate();

	std::string name;
	std::vector<class Entity*> nodes;

	std::vector<std::weak_ptr<class Mesh>> meshes;
	std::vector<std::weak_ptr<class Material>> materials;
};
