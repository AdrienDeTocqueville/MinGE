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
	void *instance; // Butterfly pointer <dependency counters...| job counter | * | system instance>
	uint32_t type_index, dependency_count;
};

static std::vector<struct system_t> systems;
static std::vector<system_type_t> system_types;

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

	Time::init();
	Random::init();
	JobSystem::init();
	Input::init(&window);
	Renderer::init();

	// Register builtin systems
	Engine::register_system_type(TransformSystem::type);
	//Engine::register_system_type(SYSTEM_TYPE(RenderSystem, 1));
}

void Engine::destroy()
{
	// TODO: destroy systems before their dependencies
	for (auto &system : systems)
	{
		system_types[system.type_index].destroy(system.instance);
		void *butterfly = (uint8_t*)system.instance - sizeof(std::atomic<int>) -
			system.dependency_count * sizeof(std::atomic<int>*);
		free(butterfly);
	}
	systems.clear();
	system_types.clear();

	JobSystem::destroy();
	//Debug::destroy();

#ifdef PROFILE
	MicroProfileSetForceEnable(false);

	MicroProfileWebServerStop();
	MicroProfileShutdown();
#endif
}


void Engine::register_system_type(const system_type_t &system_type)
{
	for (const system_type_t &type : system_types)
	{
		if (type.name == system_type.name || strcmp(type.name, system_type.name) == 0)
			return; // Already registered
	}
	system_types.push_back(system_type);
}

void *Engine::create_system(const char *type_name, const void **dependencies)
{
	for (uint32_t i(0); i < system_types.size(); i++)
	{
		if (system_types[i].name == type_name || strcmp(system_types[i].name, type_name) == 0)
		{
			int dependencies_size = system_types[i].dependency_count * sizeof(std::atomic<int>*);
			auto butterfly = (uint8_t*)malloc(dependencies_size + sizeof(std::atomic<int>) + system_types[i].size);

			auto counters = (std::atomic<int>**)butterfly;
			for (uint32_t d(0); d < system_types[i].dependency_count; d++)
				counters[d] = (std::atomic<int>*)(dependencies[d]) - 1;
			auto *counter = new(butterfly + dependencies_size) std::atomic<int>(1);
			void *instance = butterfly + dependencies_size + sizeof(std::atomic<int>);

			system_types[i].init(instance);
			systems.push_back({instance, i, system_types[i].dependency_count});
			return instance;
		}
	}
	return nullptr;
}

const system_type_t *Engine::get_system_type(void *system)
{
	for (auto &sys : systems)
	{
		if (sys.instance == system)
			return &system_types[sys.type_index];
	}
	return nullptr;
}

void sys_update(const void *data)
{
	auto sys = (system_t*)data;

	auto counters = (std::atomic<int>**)((uint8_t*)sys->instance - sizeof(std::atomic<int>));
	for (int32_t d(1); d <= (int32_t)sys->dependency_count; d++)
		JobSystem::wait(counters[-d], 1);
	void *instance = sys->instance;
	system_types[sys->type_index].update(instance);
}

void Engine::start_frame()
{
	MICROPROFILE_SCOPEI("ENGINE", "update");

	Time::tick();
	Input::update();

	// Launch system update jobs
	for (auto &sys : systems)
	{
		if (system_types[sys.type_index].update == NULL)
			continue;
		auto counter = (std::atomic<int>*)sys.instance - 1;
		*counter = 0; // This should be doable in sys_update
		JobSystem::run(sys_update, &sys, counter);
	}
}

void Engine::end_frame()
{
	// Wait for all system updates
	for (auto &sys : systems)
	{
		auto *counter = (std::atomic<int>*)sys.instance - 1;
		JobSystem::wait(counter, 1);
	}

	MicroProfileFlip();
}


void Engine::setWindowSize(vec2 _newSize)
{
	Input::setWindowSize(_newSize);
}
