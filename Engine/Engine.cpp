#include "Engine.h"
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
	JobSystem::destroy();
	//Debug::destroy();

#ifdef PROFILE
	MicroProfileSetForceEnable(false);

	MicroProfileWebServerStop();
	MicroProfileShutdown();
#endif
}

/// Methods (public)
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

/// Setters
void Engine::setWindowSize(vec2 _newSize)
{
	Input::setWindowSize(_newSize);
}