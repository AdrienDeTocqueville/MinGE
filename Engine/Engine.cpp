#include "Engine.h"
#include "Entity.h"

#include "Assets/PhysicMaterial.h"
#include "Assets/Program.h"
#include "Assets/Mesh.h"

#include "Utility/Time.h"
#include "Utility/Debug.h"
#include "Utility/IO/Input.h"

#ifdef DEBUG
	#include "Components/Component.h"
#endif

Engine* Engine::instance = nullptr;

Engine::Engine(sf::RenderWindow* _window, unsigned _FPS):
	clock(), pause(false)
{
	instance = this;

	_window->setFramerateLimit(_FPS);

	Time::init();
	Input::init(_window);

	GraphicEngine::create();
	PhysicEngine::create();
	ScriptEngine::create();

#ifdef REPORTFPS
	if (!font.loadFromFile("Resources/Calibri.ttf"))
		Error::add(WARNING, "Engine(): Cannot load font: Resources/Calibri.ttf");
	else
	{
		text.setString("");
		text.setFont(font);
		text.setFillColor(sf::Color::Black);
		text.setCharacterSize(20);
		text.setPosition(8, 0);
	}
#endif
}

Engine::~Engine()
{
	clear();

	GraphicEngine::destroy();
	PhysicEngine::destroy();
	ScriptEngine::destroy();

	Input::destroy();

	instance = nullptr;
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
	Time::deltaTime = clock.restart().asSeconds() * Time::timeScale;
	Time::time += Time::deltaTime;

	/// Update events
		Input::update();


	if (pause)
		return false;

#ifdef REPORTFPS
	sf::Clock timer;
	static float sTime = 0.0f, pTime = 0.0f, dTime = 0.0f;
#endif


	/// Update scripts
		ScriptEngine::get()->start();
		ScriptEngine::get()->update();
#ifdef REPORTFPS
		sTime += timer.restart().asSeconds();
#endif

	/// Step physic simulation
		PhysicEngine::get()->simulate();
#ifdef REPORTFPS
		pTime += timer.restart().asSeconds();
#endif

	/// Re-update scripts
		ScriptEngine::get()->lateUpdate();
#ifdef REPORTFPS
		sTime += timer.restart().asSeconds();
#endif

	/// Render scene
		GraphicEngine::get()->render();
#ifdef REPORTFPS
		dTime += timer.restart().asSeconds();
#endif



#ifdef REPORTFPS
	acu += Time::deltaTime;
	if (acu >= 1.0f)
	{
		float ratio = 1000.0f / frames;
		text.setString("FPS: " + toString<unsigned>(frames) +
					   "\nScripts: " + toString<float>(ratio * sTime) + " ms" +
					   "\nPhysics: " + toString<float>(ratio * pTime) + " ms" +
					   "\nRender:  " + toString<float>(ratio * dTime) + " ms");

		frames = 0;
		acu = 0.0f;
		sTime = pTime = dTime = 0.0f;
	}
	else
		frames++;

	Input::window->pushGLStates();
		GL::UseProgram(0);
		Input::window->draw(text);
	Input::window->popGLStates();
#endif

	return true;
}

void Engine::clear()
{
	std::cout << "Entities: "; Entity::clear();
	std::cout << "done" << std::endl;

	std::cout << "Meshes: "; Mesh::clear();
	std::cout << "done" << std::endl;

	std::cout << "Programs: "; Program::clear();
	std::cout << "done" << std::endl;

	std::cout << "Textures: "; Texture::clear();
	std::cout << "done" << std::endl;

	std::cout << "Materials: "; Material::clear();
	std::cout << "done" << std::endl;

	std::cout << "Physic materials: "; PhysicMaterial::clear();
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
	GraphicEngine::get()->updateCameraViewPort();
}

/// Getters
bool Engine::getPause() const
{
	return pause;
}
