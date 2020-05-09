#include "Core/Scene.h"
#include "Core/Engine.h"
#include "Core/Entity.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>

#include "IO/json.hpp"

using namespace nlohmann;

Scene::Scene(system_ref_t _systems[], int _system_count):
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
		if (type->serialize)
			system["data"] = type->serialize(systems[i].instance);
		system["type"] = type->name;
		system["name"] = systems[i].name;
		scene["systems"].push_back(system);
	}

	std::ofstream file(fullname.c_str(), std::ofstream::trunc);
	file << std::setw(4) << scene << std::endl;
}
