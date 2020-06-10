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

#include "IO/URI.h"
#include "IO/json.hpp"
#include "Utility/Error.h"
#include "Utility/stb_sprintf.h"

#include "Structures/Bounds.h"
#include "Graphics/Mesh/Mesh.h"

using namespace nlohmann;

Scene::Scene(int count, system_ref_t *refs):
	system_count(count), systems(refs)
{
	if (!refs && system_count)
		systems = (system_ref_t*)malloc(system_count * sizeof(*systems));
}

Scene::Scene(Scene &s)
{
	system_count = s.system_count;
	systems = s.systems;

	memset(&s, 0, sizeof(Scene));
}

Scene::~Scene()
{
	for (int i = 0; i < system_count; i++)
	{
		Engine::free_system(systems[i].instance);
		free((void*)systems[i].name);
	}
	free(systems);
}

bool Scene::save(const char *URI)
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


	uri_t uri;
	uri.parse(URI);
	if (!uri.on_disk)
		return false;

	if (const char *overwrite = uri.get("overwrite"))
	{
		struct stat buffer;
		if (stat(uri.path.c_str(), &buffer) == 0) // if file exists
		{
			if (strcmp(overwrite, "no") == 0) return false;
			else if (strcmp(overwrite, "ask") == 0)
			{
#ifdef _WIN32
				int r = MessageBoxA(nullptr, "File already exists.\nDo you want to replace it?", "Confirm save Scene", MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
				if (r == IDNO) return false;
#else
				return false;
#endif
			}
			else if (strcmp(overwrite, "save") == 0)
			{
				stbsp_snprintf(tmp_buf, sizeof(tmp_buf), "%s.ge.old", uri.path.c_str());
				rename(uri.path.c_str(), tmp_buf);
			}
		}
	}
	std::ofstream file(uri.path.c_str(), std::ios_base::binary | std::ofstream::trunc);
	file << std::setw(4) << scene << std::endl;
	return true;
}

void *Scene::get_system(const char *name)
{
	for (int i(0); i < system_count; i++)
	{
		if (systems[i].name == name || strcmp(systems[i].name, name) == 0)
			return systems[i].instance;
	}
	return NULL;
}

const char *Scene::get_system_name(void *system)
{
	for (int i(0); i < system_count; i++)
	{
		if (systems[i].instance == system)
			return systems[i].name;
	}
	return NULL;
}

Scene Scene::load(const char *URI)
{
	uri_t uri;
	uri.parse(URI);
	if (!uri.on_disk)
	{
		Error::add(Error::USER, "Default scenes not implemented.");
		return Scene(0);
	}

	std::ifstream file(uri.path);
	if (!file)
	{
		Error::add(Error::FILE_NOT_FOUND, "File not found.");
		return Scene(0);
	}

	auto scene = json::parse(file);

	// Load entities
	{
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
			auto name = it.value().contains("name") ? _strdup(it.value()["name"].get<std::string>().c_str()) : NULL;

			Entity::entities.next_slot = uid.id();
			Entity::create(name);
			Entity::entities.get<0>()[uid.id()].gen = uid.gen();
		}
		Entity::entities.next_slot = 1;
	}

	// Load meshes
	{
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

			Mesh::meshes.next_slot = uid.id();
			Mesh::load(it.value()["uri"].get<std::string>().c_str());
			Mesh::meshes.get<4>()[uid.id()] = uid.gen();
		}
		Mesh::meshes.next_slot = 1;
	}

	// Load systems
	auto j_systems = scene["systems"];
	auto res = Scene((int)j_systems.size());
	std::vector<void*> not_ready;
	not_ready.reserve(res.system_count);

	// Alloc memory
	int i = 0;
	size_t max_dependency = 0;
	for (auto system = j_systems.begin(); system != j_systems.end(); ++system, ++i)
	{
		res.systems[i].name = _strdup(system.key().c_str());
		res.systems[i].instance = Engine::alloc_system(system.value()["type"].get<std::string>().c_str());
		not_ready.push_back(res.systems[i].instance);
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
			auto me = std::find(not_ready.begin(), not_ready.end(), res.systems[i].instance);
			if (me == not_ready.end())
				continue;

			int dep_idx = 0;
			bool ready = true;
			if (system.value().contains("dependencies"))
			{
				for (auto dep : system.value()["dependencies"])
				{
					if (void *sys = res.get_system(dep.get<std::string>().c_str()))
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

			if (auto callback = Engine::get_system_type(res.systems[i].instance)->load)
			{
				ctx.version = get_or_default<int>(system.value(), "version", 0);
				ctx.dependency_count = dep_idx;
				ctx.data = system.value()["data"];
				callback(res.systems[i].instance, ctx);
			}
			not_ready.erase(me);
		}
	}

	return res;
}

bool Scene::save(const char *URI, system_ref_t systems[], int system_count)
{
	// Trick to not call Scene destructor
	auto scene = (Scene*)alloca(sizeof(Scene));
	new (scene) Scene(system_count, systems);
	return scene->save(URI);
}
