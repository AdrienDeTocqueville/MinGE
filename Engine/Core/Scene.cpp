#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>

#include "Core/Scene.h"
#include "Core/Engine.h"
#include "Core/Entity.h"
#include "Core/Serialization.h"

#include "IO/json.hpp"
#include "Utility/Error.h"
#include "Utility/stb_sprintf.h"

#include "Structures/Bounds.h"
#include "Render/Mesh/Mesh.h"

using namespace nlohmann;

Scene::Scene(Scene &&s)
{
	system_count = s.system_count;
	systems = s.systems;

	memset(&s, 0, sizeof(Scene));
}

Scene::~Scene()
{
	clear();
	free(systems);
}

bool Scene::load(const char *path)
{
	std::ifstream file(path);
	if (!file)
	{
		Error::add(Error::FILE_NOT_FOUND, "File not found.");
		return false;
	}

	auto scene = json::parse(file);

	// Load entities
	{
		uint32_t final_slot = 1;
		uint32_t max_ent = scene["max_entity"].get<uint32_t>();

		// Clear free list
		Entity::entities.init(max_ent);
		for (uint32_t i(1); i <= max_ent; i++)
			Entity::entities.get<0>(i)->destroyed = 1;

		// Populate
		auto entities = scene["entities"];
		for (auto it = entities.rbegin(); it != entities.rend(); ++it)
		{
			UID32 uid = it.value()["uint"].get<uint32_t>();
			auto name = it.value().contains("name") ? it.value()["name"].get<std::string>().c_str() : NULL;

			auto *data = Entity::entities.get<0>();
			if (uid.id() == 1) final_slot = *(uint32_t*)(data + uid.id());
			else *(uint32_t*)(data + uid.id() - 1) = *(uint32_t*)(data + uid.id());

			Entity::entities.next_slot = uid.id();
			Entity::create(name);
			Entity::entities.get<0>()[uid.id()].gen = uid.gen();
		}
		Entity::entities.next_slot = final_slot;
	}

	// Load meshes
	{
		uint32_t final_slot = 1;
		uint32_t max_mesh = scene["max_mesh"].get<uint32_t>();

		// Clear free list
		Mesh::meshes.init(max_mesh);
		for (uint32_t i(1); i <= max_mesh; i++)
			Mesh::meshes.get<2>()[i] = NULL;

		// Populate
		auto meshes = scene["meshes"];
		for (auto it = meshes.rbegin(); it != meshes.rend(); ++it)
		{
			UID32 uid = it.value()["uint"].get<uint32_t>();

			auto *data = Mesh::meshes.get<0>();
			if (uid.id() == 1) final_slot = *(uint32_t*)(data + uid.id());
			else *(uint32_t*)(data + uid.id() - 1) = *(uint32_t*)(data + uid.id());

			Mesh::meshes.next_slot = uid.id();
			Mesh::load(it.value()["uri"].get<std::string>().c_str());
			Mesh::meshes.get<4>()[uid.id()] = uid.gen();
		}
		Mesh::meshes.next_slot = final_slot;
	}

	// Load systems
	clear();
	auto j_systems = scene["systems"];
	reserve(system_count = (int)j_systems.size());

	std::vector<void*> not_ready;
	not_ready.reserve(system_count);

	// Alloc memory
	int i = 0;
	size_t max_dependency = 0;
	for (auto system = j_systems.begin(); system != j_systems.end(); ++system, ++i)
	{
		systems[i].name = strdup(system.key().c_str());
		systems[i].instance = Engine::alloc_system(system.value()["type"].get<std::string>().c_str());
		not_ready.push_back(systems[i].instance);
		if (system.value().contains("dependencies"))
			max_dependency = std::max(max_dependency, system.value()["dependencies"].size());
	}

	// Handle dependencies
	SerializationContext ctx((int)max_dependency);
	while (!not_ready.empty())
	{
		i = 0;
		for (auto system = j_systems.begin(); system != j_systems.end(); ++system, ++i)
		{
			auto me = std::find(not_ready.begin(), not_ready.end(), systems[i].instance);
			if (me == not_ready.end())
				continue;

			int dep_idx = 0;
			bool ready = true;
			if (system.value().contains("dependencies"))
			{
				for (auto dep : system.value()["dependencies"])
				{
					if (void *sys = get_system(dep.get<std::string>().c_str()))
					{
						auto depen = std::find(not_ready.begin(), not_ready.end(), sys);
						if (depen != not_ready.end())
						{
							ready = false;
							break;
						}
						else
							ctx.dependencies[dep_idx++] = sys;
					}
				}
			}

			if (!ready) continue;

			if (auto callback = Engine::get_system_type(systems[i].instance)->load)
			{
				ctx.version = get_or_default<int>(system.value(), "version", 0);
				ctx.dependency_count = dep_idx;
				ctx.data = system.value()["data"];
				callback(systems[i].instance, ctx);
			}
			not_ready.erase(me);
		}
	}

	return true;
}

bool Scene::save(const char *path, Overwrite mode) const
{
	char tmp_buf[256];
	json scene;

	// Save entities
	{
		uint32_t max_ent = 0;
		json entities = json::array();
		entities.get_ptr<nlohmann::json::array_t*>()->reserve(Entity::entities.size);
		for (uint32_t i(1); i <= Entity::entities.size; i++)
		{
			auto entity = Entity::get(i);
			if (entity == Entity::none)
				continue;

			max_ent = i;
			json entity_dump = json::object();
			entity_dump["uint"] = entity.uint();
			if (entity.name()) entity_dump["name"] = entity.name();
			entities.push_back(entity_dump);
		}

		scene["max_entity"] = max_ent;
		scene["entities"] = entities;
	}

	// Save meshes
	{
		uint32_t max_mesh = 0;
		json meshes = json::array();
		meshes.get_ptr<nlohmann::json::array_t*>()->reserve(Mesh::meshes.size);
		for (uint32_t i(1); i <= Mesh::meshes.size; i++)
		{
			auto mesh = Mesh::get(i);
			if (mesh == Mesh::none)
				continue;

			max_mesh = i;
			json mesh_dump = json::object();
			mesh_dump["uint"] = mesh.uint();
			mesh_dump["uri"] = mesh.uri();
			meshes.push_back(mesh_dump);
		}

		scene["max_mesh"] = max_mesh;
		scene["meshes"] = meshes;
	}

	// Save systems
	// TODO: multithread serialize
	scene["systems"] = json::object();
	for (int i(0); i < system_count; i++)
	{
		auto *type = Engine::get_system_type(systems[i].instance);

		json system = json::object();
		if (type->save)
		{
			SerializationContext ctx;
			type->save(systems[i].instance, ctx);

			system["data"] = ctx.data;
			if (ctx.version)
				system["version"] = ctx.version;
			if (ctx.dependency_count)
			{
				json dependencies = json::array();
				dependencies.get_ptr<nlohmann::json::array_t*>()->reserve(ctx.dependency_count);
				for (int i = 0; i < ctx.dependency_count; i++)
					dependencies.push_back(get_system_name(ctx.dependencies[i]));
				system["dependencies"] = dependencies;
			}
		}
		system["type"] = type->name;
		scene["systems"][systems[i].name] = system;
	}


	struct stat buffer;
	if (mode && stat(path, &buffer) == 0) // if file exists
	{
		switch (mode)
		{
		case Overwrite::No:
			return false;

		case Overwrite::Ask:
		{
#ifdef _WIN32
			int r = MessageBoxA(nullptr, "File already exists.\nDo you want to replace it?", "Confirm save Scene", MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
			if (r == IDNO) return false;
#else
			return false;
#endif
			break;
		}

		case Overwrite::Backup:
			stbsp_snprintf(tmp_buf, sizeof(tmp_buf), "%s.old", path);
			rename(path, tmp_buf);
			break;
		}
	}
	std::ofstream file(path, std::ios_base::binary | std::ofstream::trunc);
	file << std::setw(4) << scene << std::endl;
	return true;
}

void Scene::add_system(char *name, void *instance)
{
	reserve(system_count ? system_count * 2 : 8);
	systems[system_count].name = name;
	systems[system_count].instance = instance;
	system_count++;
}

void Scene::remove_system(const char *name)
{
	for (int i(0); i < system_count; i++)
	{
		if (strcmp(systems[i].name, name) == 0)
		{
			Engine::free_system(systems[i].instance);
			free(systems[i].name);
			system_count--;
			systems[i].name = systems[system_count].name;
			systems[i].instance = systems[system_count].instance;
			return;
		}
	}
}

void Scene::clear()
{
	for (int i = 0; i < system_count; i++)
	{
		Engine::free_system(systems[i].instance);
		free(systems[i].name);
	}
	system_count = 0;
}

void *Scene::get_system(const char *name) const
{
	for (int i(0); i < system_count; i++)
	{
		if (systems[i].name == name || strcmp(systems[i].name, name) == 0)
			return systems[i].instance;
	}
	return NULL;
}

const char *Scene::get_system_name(void *system) const
{
	for (int i(0); i < system_count; i++)
	{
		if (systems[i].instance == system)
			return systems[i].name;
	}
	return NULL;
}

void Scene::reserve(int count)
{
	if (count > capacity)
	{
		capacity = count;
		systems = (system_ref_t*)realloc(systems, capacity * sizeof(system_ref_t));
	}
}
