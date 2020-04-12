#include "Core/Engine.h"
#include "ECS/Entity.h"

#include "Renderer/GLDriver.h"

#include "Utility/Time.h"
//#include "Utility/Debug.h"
#include "Utility/Random.h"
#include "Utility/IO/Input.h"
#include "Utility/JobSystem/JobSystem.h"

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
	//Debug::init();
	Random::init();
	JobSystem::init();
	Input::init(&window);
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
			auto *counter = new(butterfly + dependencies_size) std::atomic<int>(0);
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


void Engine::start_frame()
{
	MICROPROFILE_SCOPEI("ENGINE", "update");

	Time::tick();
	Input::update();
}

void Engine::end_frame()
{
	MicroProfileFlip();
}


void Engine::setWindowSize(vec2 _newSize)
{
	Input::setWindowSize(_newSize);
}
