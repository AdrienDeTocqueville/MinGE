#include "Core/Engine.h"
#include "Core/Entity.h"

#include "Transform/Transform.h"

#include "Renderer/GLDriver.h"

#include "Utility/Time.h"
#include "Math/Random.h"
#include "JobSystem/JobSystem.inl"
#include "IO/Input.h"
#include "Renderer/Renderer.h"

//#define MICROPROFILE_MAX_FRAME_HISTORY (2<<10)
#define MICROPROFILE_IMPL
#include "Profiler/profiler.h"

uint32_t Entity::next_index = 1;

struct system_t
{
	// before: std::atomic<int> *dependencies[dependency_count]
	std::atomic<int> job;
	uint32_t type_index, dependency_count;
	// after: <system instance>

	inline void *instance() { return this + 1; }
};

static std::vector<system_type_t> system_types;
static std::vector<system_t*> systems;
static size_t first_on_main_thread;

void Engine::init(sf::RenderWindow &window, unsigned _FPS)
{
	window.setFramerateLimit(_FPS);

#ifdef PROFILE
	MicroProfileOnThreadCreate("Main");

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
	Renderer::init();

	// Register builtin systems
	Engine::register_system_type(TransformSystem::type);
	//Engine::register_system_type(SYSTEM_TYPE(RenderSystem, 1));
}


static inline void sys_destroy(const void *data)
{
	auto sys = *(system_t**)data;

	if (auto destroy = system_types[sys->type_index].destroy)
	{
		// Wait for system depending on this one to remove their veto
		JobSystem::wait(&sys->job, 0);
		// Run callback
		destroy(sys->instance());
	}
	// Remove veto from dependencies
	auto dependencies = (std::atomic<int>**)sys;
	for (int32_t i(1); i <= (int32_t)sys->dependency_count; i++)
		dependencies[-i]->fetch_add(1, std::memory_order_relaxed);
}

void Engine::destroy()
{
	for (system_t *system : systems)
		system->job = 0;
	for (system_t *system : systems)
	{
		// Add veto to dependencies
		auto dependencies = (std::atomic<int>**)system;
		for (int32_t i(1); i <= (int32_t)system->dependency_count; i++)
			dependencies[-i]->fetch_sub(1, std::memory_order_relaxed);
	}

	size_t i(0);

	// Launch jobs
	for (; i < first_on_main_thread; i++)
		JobSystem::run(sys_destroy, &systems[i], &systems[i]->job);

	// Exec needed on main thread
	for (; i < systems.size(); i++)
	{
		sys_destroy(&systems[i]);
		systems[i]->job.fetch_add(1, std::memory_order_relaxed);
	}

	// Sync
	for (system_t *system : systems)
		JobSystem::wait(&system->job, 1);

	// Free allocated memory
	for (system_t *system : systems)
	{
		int dependencies_size = system_types[system->type_index].dependency_count * sizeof(std::atomic<int>*);
		auto butterfly = (uint8_t*)system - dependencies_size;
		free(butterfly);
	}

	systems.clear();
	system_types.clear();

	JobSystem::destroy();

#ifdef PROFILE
	MicroProfileSetForceEnable(false);

	MicroProfileWebServerStop();
	MicroProfileShutdown();
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

void *Engine::create_system(const char *type_name, const void **dependencies)
{
	auto type = get_type(type_name);
	if (type == NULL)
		return NULL;

	int dependencies_size = type->dependency_count * sizeof(std::atomic<int>*);
	auto butterfly = (uint8_t*)malloc(dependencies_size + sizeof(system_t) + type->size);
	auto system = (system_t*)(butterfly + dependencies_size);

	auto counters = (std::atomic<int>**)butterfly;
	for (uint32_t d(0); d < type->dependency_count; d++)
		counters[d] = (std::atomic<int>*)(dependencies[d]) - 1;
	new(&system->job) std::atomic<int>();
	system->type_index = (type - system_types.data()) / sizeof(*type);
	system->dependency_count = type->dependency_count;
	type->init(system->instance());

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
	system_t *sys = (system_t*)system - 1;
	return &system_types[sys->type_index];
}


struct update_data_t
{
	system_t *sys;
	void (*update)(void *instance);
};

static inline void sys_update(const void *data)
{
	auto d = *(update_data_t*)data;

	// Wait for dependencies
	auto dependencies = (std::atomic<int>**)d.sys;
	for (int32_t i(1); i <= (int32_t)d.sys->dependency_count; i++)
		JobSystem::wait(dependencies[-i], 1);
	// Run callback
	d.update(d.sys->instance());
}

void Engine::start_frame()
{
	MICROPROFILE_SCOPEI("ENGINE", "update");

	Time::tick();
	Input::update();

	for (system_t *system : systems)
		system->job = 0;

	size_t i = 0;

	// Launch system update jobs
	for (; i < first_on_main_thread; i++)
	{
		update_data_t data;
		if (data.update = system_types[systems[i]->type_index].update)
		{
			data.sys = systems[i];
			JobSystem::run(sys_update, &data, &systems[i]->job);
		}
		else
			systems[i]->job.fetch_add(1, std::memory_order_relaxed);
	}
	// Exec needed systems on main thread
	for (; i < systems.size(); i++)
	{
		update_data_t data;
		if (data.update = system_types[systems[i]->type_index].update)
		{
			data.sys = systems[i];
			sys_update(&data);
		}
		systems[i]->job.fetch_add(1, std::memory_order_relaxed);
	}
}

void Engine::end_frame()
{
	// Wait for all system updates
	for (system_t *system : systems)
		JobSystem::wait(&system->job, 1);

	MicroProfileFlip();
}


void Engine::setWindowSize(vec2 _newSize)
{
	Input::setWindowSize(_newSize);
}
