#include "Render/GLDriver.h"

#define MICROPROFILE_IMPL
#define MICROPROFILEUI_IMPL
#define MICROPROFILEDRAW_IMPL
#define MICROPROFILE_GPU_TIMERS_GL 1
#include "Profiler/profiler.h"

#include "Core/Engine.h"
#include "Core/Entity.h"
#include "Core/AssetDatabase.h"

#include "IO/json.hpp"
#include "IO/Input.h"
#include "IO/FileSystem.h"
#include "JobSystem/JobSystem.inl"
#include "Math/Random.h"
#include "Render/RenderEngine.h"
#include "Utility/Log.h"
#include "Utility/Time.h"
#include "Utility/stb_sprintf.h"


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

#include "Core/Entity.inl"

/// Engine
void Engine::init(int argc, char **argv, struct SDL_Window *window)
{
	Time::init();
	Random::init();
	JobSystem::init();
	Input::init(window);

	AssetDatabase::init(argc, argv);
	RenderEngine::init();
}

void Engine::load()
{
	RenderEngine::load();
}

void Engine::clear()
{
	/*
	 * System destruction in jobs is disabled for now
	 * RenderEngine::remove_buffer would need synchronization
	 * GL assets destruction can only be done on main thread

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
	*/

	// Destroy systems
	for (system_t *system : systems)
	{
		if (auto callback = system_types[system->type_index].destroy)
			callback(system->instance());
		free(system);
	}
	systems.clear();
	system_types.clear();

	RenderEngine::clear();
}

void Engine::destroy()
{
	Engine::clear();

	RenderEngine::destroy();
	JobSystem::destroy();

	AssetDatabase::destroy();
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
void Engine::register_system_type(const system_type_t &type)
{
	if (system_type(type.name) == NULL)
		system_types.emplace_back(type);
}

const system_type_t *Engine::system_type(const char *type_name)
{
	for (system_type_t &type : system_types)
	{
		if (type.name == type_name || strcmp(type.name, type_name) == 0)
			return &type;
	}
	return NULL;
}

const system_type_t *Engine::system_type(const void *instance)
{
	system_t *sys = system_t::from_instance(instance);
	return &system_types[sys->type_index];
}

void *Engine::alloc_system(const char *type_name)
{
	const system_type_t *type = system_type(type_name);
	ASSERT(type != nullptr, "No system type named '" << type_name << "'");

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

// Input
void Input::on_resize_window_event()
{
	for (system_t *system : systems)
	{
		if (auto callback = system_types[system->type_index].on_resize_window)
			callback(system->instance());
	}
}
