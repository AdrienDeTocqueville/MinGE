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
	uint32_t type_index;
	JobSystem::Semaphore counter;
	IF_PROFILE(MicroProfileToken token;)
	// after: <system instance>

	static inline system_t *from_instance(const void *i) { return (system_t*)i - 1; }
	inline void *instance() { return this + 1; }
};

static std::vector<system_type_t> system_types;
static std::vector<system_t*> systems;

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

	Time::init();
	Random::init();
	JobSystem::init();
	Input::init(&window);
	RenderEngine::init();

	// Register builtin systems
	Engine::register_system_type(TransformSystem::type);
	Engine::register_system_type(GraphicsSystem::type);
}

void Engine::destroy()
{
	// Launch system detroy jobs
	{ MICROPROFILE_SCOPEI("ENGINE", "launch_destroy_jobs");
	for (system_t *system : systems)
	{
		if (auto callback = system_types[system->type_index].destroy)
			JobSystem::run(callback, system->instance(), &system->counter IF_PROFILE(, system->token));
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

void *Engine::alloc_system(const char *type_name)
{
	auto type = get_type(type_name);
	assert(type && "No such system type");

	auto system = (system_t*)malloc(sizeof(system_t) + type->size);
	system->type_index = (uint32_t)(type - system_types.data());
	new (&system->counter) JobSystem::Semaphore();

#ifdef PROFILE
	static char sys_name[256];
	snprintf(sys_name, sizeof(sys_name), "%s %d", type_name, systems.size());
	system->token = MicroProfileGetToken(type_name, sys_name, -1);
#endif

	systems.push_back(system);
	return system->instance();
}

const system_type_t *Engine::get_system_type(void *system)
{
	system_t *sys = system_t::from_instance(system);
	return &system_types[sys->type_index];
}


void Engine::update()
{
	{ MICROPROFILE_SCOPEI("ENGINE", "update");

	Time::tick();
	Input::poll_events();

	// Launch system update jobs
	{ MICROPROFILE_SCOPEI("ENGINE", "launch_update_jobs");
	for (system_t *system : systems)
	{
		if (auto callback = system_types[system->type_index].update)
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
	}

	MicroProfileFlip();
}
