#include "Engine.h"
#include "Entity.h"

#include "Systems/GraphicEngine.h"
#include "Systems/PhysicEngine.h"
#include "Systems/ScriptEngine.h"

#include "Assets/Texture.h"
#include "Assets/Program.h"

#include "Utility/Time.h"
#include "Utility/Debug.h"
#include "Utility/IO/Input.h"
#include "Utility/JobSystem/JobSystem.h"

//#define MICROPROFILE_MAX_FRAME_HISTORY (2<<10)
#define MICROPROFILE_IMPL
#include "Profiler/profiler.h"

#ifdef DEBUG
#include "Components/Component.h"
#endif

Engine* Engine::instance = nullptr;

Engine::Engine(sf::RenderWindow* _window, unsigned _FPS):
	clock(), pause(false)
{
	instance = this;

	_window->setFramerateLimit(_FPS);

#ifdef PROFILE
	MicroProfileOnThreadCreate("Main");

	MicroProfileSetForceEnable(true);
	MicroProfileSetEnableAllGroups(true);
	MicroProfileSetForceMetaCounters(true);

	MicroProfileWebServerStart();
#endif

	Time::init();
	JobSystem::init();
	Input::init(_window);

	GraphicEngine::create();
	PhysicEngine::create();
	ScriptEngine::create();
}

Engine::~Engine()
{
	clear();

	GraphicEngine::destroy();
	PhysicEngine::destroy();
	ScriptEngine::destroy();

	Input::destroy();
	JobSystem::destroy();

	instance = nullptr;

#ifdef PROFILE
	MicroProfileSetForceEnable(false);

	MicroProfileWebServerStop();
	MicroProfileShutdown();
#endif
}

/// Methods (public)
void Engine::start()
{
	clock.restart();

	if (Error::check())
	{
		delete this;

		exit(EXIT_FAILURE);
	}

	Debug::init();

	ScriptEngine::get()->start();

	GLenum err;
	while ( ( err = glGetError() ) != GL_NO_ERROR) {
		std::cout << "OpenGL error detected: " << err << std::endl;
	}

#ifdef DEBUG
	float loadTime = clock.restart().asSeconds();
	std::cout << "Loaded in " << loadTime << " seconds" << std::endl;
#endif

	clock.restart();
}

bool Engine::update()
{
	MICROPROFILE_SCOPEI("ENGINE", "update");

	Time::deltaTime = clock.restart().asSeconds() * Time::timeScale;
	Time::time += Time::deltaTime;

	/// Update events
	Input::update();

	if (pause)
		return false;

	/// Update scripts
	ScriptEngine::get()->start();
	ScriptEngine::get()->update();

	/// Step physic simulation
	PhysicEngine::get()->simulate();

	/// Re-update scripts
	ScriptEngine::get()->lateUpdate();

	/// Render scene
	GraphicEngine::get()->render();


	MicroProfileFlip();

	return true;
}

void Engine::clear()
{
	MICROPROFILE_SCOPEI("ENGINE", "clear");

	std::cout << "Entities: "; Entity::clear();
	std::cout << "done" << std::endl;

	std::cout << "Programs: "; Program::clear();
	std::cout << "done" << std::endl;

	std::cout << "Textures: "; Texture::clear();
	std::cout << "done" << std::endl;

	GraphicEngine::get()->clear();
	PhysicEngine::get()->clear();
	ScriptEngine::get()->clear();

	Debug::destroy();


#ifdef DEBUG
	if (Component::instances != 0)
	{
		Component::instances = 0;
		Error::add(WARNING, "One or more component have not been deleted ("+toString(Component::instances)+")");
		exit(EXIT_FAILURE);
	}
#endif // DEBUG
}

/// Setters
void Engine::setPause(bool _pause)
{
	pause = _pause;
}

void Engine::togglePause()
{
	pause = !pause;
}

void Engine::setWindowSize(vec2 _newSize)
{
	Input::setWindowSize(_newSize);
}

/// Getters
bool Engine::getPause() const
{
	return pause;
}
