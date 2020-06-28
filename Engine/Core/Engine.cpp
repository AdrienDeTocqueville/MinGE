#include "Render/GLDriver.h"

#define MICROPROFILE_IMPL
#define MICROPROFILEUI_IMPL
#define MICROPROFILEDRAW_IMPL
#define MICROPROFILE_GPU_TIMERS_GL 1
#include "Profiler/profiler.h"

#include "Core/Engine.h"
#include "Core/Entity.h"
#include "Core/Asset.h"
#include "Core/Scene.h"

#include "IO/json.hpp"
#include "IO/Input.h"
#include "JobSystem/JobSystem.inl"
#include "Math/Random.h"
#include "Render/RenderEngine.h"
#include "Utility/Time.h"
#include "Utility/stb_sprintf.h"


const Entity Entity::none;
multi_array_t<entity_t> Entity::entities;

struct system_t
{
	uint32_t type_index;
	std::atomic<uint32_t> readers;
	decltype(system_type_t::update) update;
	JobSystem::Semaphore counter;
	IF_PROFILE(MicroProfileToken token);
	// after: <system instance>

	static inline system_t *from_instance(const void *i) { return (system_t*)i - 1; }
	inline void *instance() { return this + 1; }
};

static std::vector<system_type_t> system_types;
static std::vector<system_t*> systems;

static std::vector<asset_type_t> asset_types;


/// Entity
void Entity::destroy()
{
	assert(is_valid() && "Invalid entity");

	for (system_t *system : systems)
	{
		if (auto callback = system_types[system->type_index].on_destroy_entity)
			callback(system->instance(), *this);
	}

	auto *e = entities.get<0>(id());
	e->gen++;
	e->destroyed = 1;
	free(e->name);
	entities.remove(id());
}

Entity Entity::create(const char *name)
{
	auto idx = entities.add();
	auto *e = entities.get<0>(idx);
	e->destroyed = 0;
	e->name = name ? strdup(name) : NULL;
	return Entity(idx, e->gen);
}

Entity Entity::get(uint32_t i)
{
	if (entities.get<0>(i)->destroyed)
		return Entity::none;
	return Entity(i, entities.get<0>(i)->gen);
}

Entity Entity::get(const char *name)
{
	for (uint32_t i = 1; i <= entities.size; i++)
	{
		const char *n = entities.get<0>(i)->name;
		if ((entities.get<0>(i)->destroyed == 1) | (n == NULL))
			continue;
		if (n == name || strcmp(n, name) == 0)
			return Entity::get(i);
	}
	return Entity::none;
}

void Entity::clear()
{
	for (uint32_t i(1); i <= Entity::entities.size; i++)
	{
		if (!Entity::entities.get<0>(i)->destroyed)
			free(entities.get<0>(i)->name);
	}
	Entity::entities.clear();
}


/// Engine
void Engine::init(struct SDL_Window *window)
{
	Time::init();
	Random::init();
	JobSystem::init();
	Input::init(window);
	RenderEngine::init();
}

void Engine::destroy()
{
	Engine::clear();

	RenderEngine::destroy();
	JobSystem::destroy();
}

void Engine::clear()
{
	// Launch system detroy jobs
	{ MICROPROFILE_SCOPEI("ENGINE", "launch_destroy_jobs");
	for (system_t *system : systems)
	{
		if (auto callback = system_types[system->type_index].destroy)
#ifdef PROFILE
			JobSystem::run(callback, system->instance(), &system->counter, system->token);
#else
			JobSystem::run(callback, system->instance(), &system->counter);
#endif
	}
	}

	// Wait for all systems
	{ MICROPROFILE_SCOPEI("ENGINE", "wait_jobs");
	for (system_t *system : systems)
		system->counter.wait();
	}

	// Free allocated memory
	for (system_t *system : systems)
		free(system);

	systems.clear();
	system_types.clear();

	// Destroy entities
	Entity::clear();
}

void Engine::frame()
{
	MICROPROFILE_SCOPEI("ENGINE", "frame");

	Time::tick();
	Input::poll_events();
	RenderEngine::start_frame();

	// Launch system update jobs
	{ MICROPROFILE_SCOPEI("ENGINE", "launch_update_jobs");
	for (system_t *system : systems)
	{
		if (auto callback = system->update)
#ifdef PROFILE
			JobSystem::run(callback, system->instance(), &system->counter, system->token);
#else
			JobSystem::run(callback, system->instance(), &system->counter);
#endif
	}
	}

	// Wait for all systems
	{ MICROPROFILE_SCOPEI("ENGINE", "wait_jobs");
	for (system_t *system : systems)
		system->counter.wait();
	}

	RenderEngine::flush();
	JobSystem::sleep();
}


// systems
static inline system_type_t *get_type(const char *name)
{
	for (system_type_t &type : system_types)
	{
		if (type.name == name || strcmp(type.name, name) == 0)
			return &type;
	}
	return NULL;
}

void Engine::register_system_type(const system_type_t &system_type)
{
	if (get_type(system_type.name) == NULL)
		system_types.emplace_back(system_type);
}

const system_type_t *Engine::get_system_type(void *system)
{
	system_t *sys = system_t::from_instance(system);
	return &system_types[sys->type_index];
}

void *Engine::alloc_system(const char *type_name)
{
	auto type = get_type(type_name);
	assert(type && "No such system type");

	auto system = (system_t*)malloc(sizeof(system_t) + type->size);
	system->type_index = (uint32_t)(type - system_types.data());
	system->update = type->update;
	new (&system->readers) std::atomic<uint32_t>{0};
	new (&system->counter) JobSystem::Semaphore();

#ifdef PROFILE
	static char sys_name[256];
	stbsp_snprintf(sys_name, sizeof(sys_name), "%s %zd", type_name, systems.size());
	system->token = MicroProfileGetToken(type_name, sys_name, -1);
#endif

	systems.emplace_back(system);
	return system->instance();
}

void Engine::free_system(void *system)
{
	system_t *sys = system_t::from_instance(system);
	for (int i = 0; i < systems.size(); i++)
	{
		if (systems[i] == sys)
		{
			if (auto callback = system_types[sys->type_index].destroy)
				callback(system);
			free(sys);
			systems[i] = systems.back();
			systems.pop_back();
			return;
		}
	}
}

void Engine::read_lock(void *system)
{
	auto *readers = &system_t::from_instance(system)->readers;

	while (true)
	{
		uint32_t prev_readers = *readers;
		if (prev_readers != -1)
		{
			uint32_t new_readers = prev_readers + 1;
			if (readers->compare_exchange_weak(prev_readers, new_readers))
				return;
		}

		JobSystem::yield();
	}
}

void Engine::read_unlock(void *system)
{
	system_t::from_instance(system)->readers--;
}

void Engine::write_lock(void *system)
{
	auto *readers = &system_t::from_instance(system)->readers;

	while (true)
	{
		uint32_t prev_readers = 0;
		if (readers->compare_exchange_weak(prev_readers, -1))
			return;

		JobSystem::yield();
	}
}

void Engine::write_unlock(void *system)
{
	system_t::from_instance(system)->readers = 0;
}

// assets
void Engine::register_asset_type(const asset_type_t &asset_type)
{
	asset_types.emplace_back(asset_type);
}

void Scene::load_assets(nlohmann::json &scene)
{
	auto &assets = scene["assets"];
	for (int i(0); i < asset_types.size(); i++)
	{
		if (asset_types[i].load && assets.contains(asset_types[i].name))
			asset_types[i].load(assets[asset_types[i].name]);
	}
}

void Scene::save_assets(nlohmann::json &scene) const
{
	scene["assets"] = nlohmann::json::object();
	for (int i(0); i < asset_types.size(); i++)
	{
		if (asset_types[i].save)
		{
			nlohmann::json asset = nlohmann::json::object();
			asset_types[i].save(asset);
			scene["assets"][asset_types[i].name].swap(asset);
		}
	}
}


// Input
void Input::on_resize_window_event()
{
	for (system_t *system : systems)
	{
		if (auto callback = system_types[system->type_index].on_resize_window)
			callback(system->instance());
	}
}