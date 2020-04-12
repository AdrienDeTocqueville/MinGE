#include "Core/Scene.h"
#include "Core/Engine.h"
#include "ECS/Entity.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iomanip>

#include "Utility/IO/json.hpp"

using namespace nlohmann;

Scene::Scene(int _system_count, system_ref_t _systems[]):
	system_count(_system_count)
{
	size_t array_size = system_count * sizeof(*systems);
	systems = (system_ref_t*)malloc(array_size);
	memcpy(systems, _systems, array_size);
}

Scene::~Scene()
{
	free(systems);
}

void Scene::save(const char *path, const char *name, bool backup_existing)
{
	std::string base(path);
	base += name;
	std::string fullname = base + ".ge";

	if (backup_existing)
	{
		std::string backup = base + "_old.ge";
		rename(fullname.c_str(), backup.c_str());
	}

	json scene;
	scene["engine"] = {
		{ "next_entity", Entity::next_index}
	};

	// TODO: serialize systems after their dependencies
	scene["systems"] = json::array();
	for (int i(0); i < system_count; i++)
	{
		auto *type = Engine::get_system_type(systems[i].instance);

		json system;
		system["type"] = type->name;
		system["name"] = systems[i].name;
		system["data"] = type->serialize(systems[i].instance);
		scene["systems"].push_back(system);
	}

	std::ofstream file(fullname.c_str(), std::ofstream::trunc);
	file << std::setw(4) << scene << std::endl;
}
