#define MICROPROFILE_IMPL
#include "Profiler/profiler.h"

#include "Core/Engine.h"
#include "Core/Entity.h"

#include "Transform/Transform.h"

#include "Graphics/Graphics.h"
#include "Graphics/GLDriver.h"
#include "Graphics/RenderEngine.h"

#include "Utility/Time.h"
#include "Math/Random.h"
#include "JobSystem/JobSystem.inl"
#include "IO/Input.h"

uint32_t Entity::next_index = 1;
const Entity Entity::none;

struct system_t
{
	// before: std::atomic<int> *dependencies[dependency_count]
	std::atomic<int> job;
	uint32_t type_index, dependency_count;
	PROFILER(MicroProfileToken token;)
	// after: <system instance>

	static inline system_t *from_instance(const void *i) { return (system_t*)i - 1; }
	inline void *instance() { return this + 1; }
	inline std::atomic<int> **dependencies() const { return (std::atomic<int>**)this - dependency_count; }
	inline std::atomic<int> **dependencies_end() const { return (std::atomic<int>**)this; }
};

static std::vector<system_type_t> system_types;
static std::vector<system_t*> systems;
static size_t first_on_main_thread;

void Engine::init(sf::RenderWindow &window, unsigned _FPS)
{
	window.setFramerateLimit(_FPS);

#ifdef PROFILE
	MicroProfileOnThreadCreate("main thread");

	MicroProfileSetForceEnable(true);
	MicroProfileSetEnableAllGroups(true);
	MicroProfileSetForceMetaCounters(true);

	MicroProfileWebServerStart();
#endif

	first_on_main_thread = 0;

	Time::init();
	Random::init();
	JobSystem::init();
	Input::init(&window);
	RenderEngine::init();

	// Register builtin systems
	Engine::register_system_type(TransformSystem::type);
	Engine::register_system_type(GraphicsSystem::type);
}


static inline void sys_destroy(system_t **data)
{
	system_t *sys = *data;

	// Run callback
	if (auto destroy = system_types[sys->type_index].destroy)
		destroy(sys->instance());
	// Remove dependency from dependencies
	auto dependencies = sys->dependencies();
	while (dependencies != sys->dependencies_end())
		JobSystem::remove_dependency(*dependencies++);
}

void Engine::destroy()
{
	// Here we have to put a dependency on each system dependencies so that
	// a system doesn't get destroyed after its dependencies
	for (system_t *system : systems)
	{
		// Add dependency to dependencies
		auto dependencies = system->dependencies();
		while (dependencies != system->dependencies_end())
			JobSystem::add_dependency(*dependencies++);
	}

	size_t i(0);

	// Launch jobs
	for (; i < first_on_main_thread; i++)
	{
		// systems[i]->job == 0 means all systems depending on this one have finished
		// so this one can be destroyed
		// use auto-decrement after so that end condition is -1
		JobSystem::run_child(sys_destroy, &systems[i], &systems[i]->job, &systems[i]->job);
	}

	// Exec needed on main thread
	for (; i < systems.size(); i++)
	{
		JobSystem::wait(&systems[i]->job);
		sys_destroy(&systems[i]);
		// manually decrement so that end condition is -1
		JobSystem::remove_dependency(&systems[i]->job);
	}

	// Sync
	for (system_t *system : systems)
		JobSystem::wait(&system->job, -1);

	// Free allocated memory
	for (system_t *system : systems)
	{
		int dependencies_size = system->dependency_count * sizeof(std::atomic<int>*);
		auto butterfly = (uint8_t*)system - dependencies_size;
		free(butterfly);
	}

	systems.clear();
	system_types.clear();

	RenderEngine::destroy();
	JobSystem::destroy();

#ifdef PROFILE
	MicroProfileShutdown();

	MicroProfileOnThreadExit();
#endif
}

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
		system_types.push_back(system_type);
}

void *Engine::alloc_system(const char *type_name, const void **dependencies, uint32_t dependency_count)
{
	auto type = get_type(type_name);
	assert(type && "No such system type");

	int dependencies_size = dependency_count * sizeof(std::atomic<int>*);
	auto butterfly = (uint8_t*)malloc(dependencies_size + sizeof(system_t) + type->size);
	auto system = (system_t*)(butterfly + dependencies_size);

	auto counters = (std::atomic<int>**)butterfly;
	for (uint32_t d(0); d < dependency_count; d++)
		counters[d] = &system_t::from_instance(dependencies[d])->job;

	new(&system->job) std::atomic<int>(0);
	system->type_index = (uint32_t)(type - system_types.data());
	system->dependency_count = dependency_count;

#ifdef PROFILE
	static char sys_name[256];
	snprintf(sys_name, sizeof(sys_name), "%s %d", type_name, systems.size());
	system->token = MicroProfileGetToken(type_name, sys_name, -1);
#endif

	if (type->on_main_thread)
		systems.push_back(system);
	else
	{
		systems.insert(systems.begin() + first_on_main_thread, system);
		first_on_main_thread++;
	}
	return system->instance();
}

const system_type_t *Engine::get_system_type(void *system)
{
	system_t *sys = system_t::from_instance(system);
	return &system_types[sys->type_index];
}


struct update_data_t
{
	system_t *sys;
	void (*update)(void *instance);
};

static inline void sys_update(update_data_t *data)
{
	MICROPROFILE_SCOPE_TOKEN(data->sys->token);
	data->update(data->sys->instance());
}

void Engine::start_frame()
{
	MICROPROFILE_SCOPEI("ENGINE", "start_frame");

	Time::tick();
	Input::update();

	for (system_t *system : systems)
		JobSystem::add_dependency(&system->job);

	size_t i = 0;

	// Launch system update jobs
	{ MICROPROFILE_SCOPEI("ENGINE", "launch_update_jobs");
	for (; i < first_on_main_thread; i++)
	{
		update_data_t data;
		if (data.update = system_types[systems[i]->type_index].update)
		{
			data.sys = systems[i];
			JobSystem::run_child(sys_update, &data, systems[i]->dependencies(), systems[i]->dependency_count, &systems[i]->job);
		}
		else
			JobSystem::remove_dependency(&systems[i]->job);
	}
	}

	// Exec needed systems on main thread
	{ MICROPROFILE_SCOPEI("ENGINE", "update_main_thread");
	for (; i < systems.size(); i++)
	{
		update_data_t data;
		if (data.update = system_types[systems[i]->type_index].update)
		{
			auto dependencies = systems[i]->dependencies();
			while (dependencies != systems[i]->dependencies_end())
				JobSystem::wait(*dependencies++);
			data.sys = systems[i];
			sys_update(&data);
		}
		JobSystem::remove_dependency(&systems[i]->job);
	}
	}
}

void Engine::end_frame()
{
	// Wait for all system updates
	{ MICROPROFILE_SCOPEI("ENGINE", "wait_update_jobs");
	for (system_t *system : systems)
		JobSystem::wait(&system->job);
	}

	MicroProfileFlip();
}
