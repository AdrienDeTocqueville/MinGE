#include "Profiler/profiler.h"

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

using namespace nlohmann;

int Scene::sys_count = 0, Scene::capacity = 0;
Scene::system_ref_t *Scene::sys = NULL;

bool Scene::load(const char *path)
{
	MICROPROFILE_SCOPEI("SCENE", "load");

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
		auto *slots = Entity::entities.get<0>();
		for (auto it = entities.rbegin(); it != entities.rend(); ++it)
		{
			UID32 uid = it.value()["uint"].get<uint32_t>();

			if (uid.id() == 1) final_slot = *(uint32_t*)(slots + uid.id());
			else *(uint32_t*)(slots + uid.id() - 1) = *(uint32_t*)(slots + uid.id());

			Entity::entities.next_slot = uid.id();
			if (it.value().contains("name"))
				Entity::create(it.value()["name"].get<std::string>().c_str());
			else
				Entity::create(NULL);

			Entity::entities.get<0>()[uid.id()].gen = uid.gen();
		}
		Entity::entities.next_slot = final_slot;
	}

	// Load assets
	Scene::load_assets(scene);
	Engine::load();

	// Load systems
	auto j_systems = scene["systems"];
	Scene::reserve(sys_count = (int)j_systems.size());

	std::vector<void*> not_ready;
	not_ready.reserve(sys_count);

	// Alloc memory
	int i = 0;
	size_t max_dependency = 0;
	for (auto system = j_systems.begin(); system != j_systems.end(); ++system, ++i)
	{
		sys[i].name = strdup(system.key().c_str());
		sys[i].instance = Engine::alloc_system(system.value()["type"].get<std::string>().c_str());
		not_ready.push_back(sys[i].instance);
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
			auto me = std::find(not_ready.begin(), not_ready.end(), sys[i].instance);
			if (me == not_ready.end())
				continue;

			int dep_idx = 0;
			bool ready = true;
			if (system.value().contains("dependencies"))
			{
				for (auto dep : system.value()["dependencies"])
				{
					if (void *s = Scene::system(dep.get<std::string>().c_str()))
					{
						auto depen = std::find(not_ready.begin(), not_ready.end(), s);
						if (depen != not_ready.end())
						{
							ready = false;
							break;
						}
						else
							ctx.dependencies[dep_idx++] = s;
					}
				}
			}

			if (!ready) continue;

			if (auto callback = Engine::system_type(sys[i].instance)->load)
			{
				ctx.version = get_or_default<int>(system.value(), "version", 0);
				ctx.dependency_count = dep_idx;
				ctx.data = system.value()["data"];
				callback(sys[i].instance, ctx);
			}
			not_ready.erase(me);
		}
	}

	return true;
}

bool Scene::save(const char *path, Overwrite mode)
{
	MICROPROFILE_SCOPEI("SCENE", "save");

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
		scene["entities"].swap(entities);
	}

	// Save assets
	Scene::save_assets(scene);

	// Save systems
	// TODO: multithread serialize
	scene["systems"] = json::object();
	for (int i(0); i < sys_count; i++)
	{
		auto *type = Engine::system_type(sys[i].instance);

		json system = json::object();
		if (type->save)
		{
			SerializationContext ctx;
			type->save(sys[i].instance, ctx);

			system["data"] = ctx.data;
			if (ctx.version)
				system["version"] = ctx.version;
			if (ctx.dependency_count)
			{
				json dependencies = json::array();
				dependencies.get_ptr<nlohmann::json::array_t*>()->reserve(ctx.dependency_count);
				for (int i = 0; i < ctx.dependency_count; i++)
					dependencies.push_back(system_name(ctx.dependencies[i]));
				system["dependencies"] = dependencies;
			}
		}
		system["type"] = type->name;
		scene["systems"][sys[i].name].swap(system);
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
			auto a = Error::ask(Error::WARNING, "File already exists.\nDo you want to replace it?");
			if (a == Error::Cancel) return false;
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
	reserve(sys_count ? sys_count * 2 : 8);
	set_system_rec(name, instance);
}

void Scene::remove_system(const char *name)
{
	for (int i(0); i < sys_count; i++)
	{
		if (strcmp(sys[i].name, name) == 0)
		{
			Engine::free_system(sys[i].instance);
			free(sys[i].name);
			sys_count--;
			sys[i].name = sys[sys_count].name;
			sys[i].instance = sys[sys_count].instance;
			return;
		}
	}
}

void Scene::clear()
{
	for (int i = 0; i < sys_count; i++)
	{
		Engine::free_system(sys[i].instance);
		free(sys[i].name);
	}
	sys_count = 0;
}

void *Scene::system(const char *name)
{
	for (int i(0); i < sys_count; i++)
	{
		if (sys[i].name == name || strcmp(sys[i].name, name) == 0)
			return sys[i].instance;
	}
	return NULL;
}

const char *Scene::system_name(void *system)
{
	for (int i(0); i < sys_count; i++)
	{
		if (sys[i].instance == system)
			return sys[i].name;
	}
	return NULL;
}

void Scene::reserve(int count)
{
	if (count > capacity)
	{
		capacity = count;
		sys = (system_ref_t*)realloc(sys, capacity * sizeof(system_ref_t));
	}
}
