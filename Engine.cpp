#include "Engine.h"
#include "Entity.h"

#include "Assets/PhysicMaterial.h"
#include "Assets/Mesh.h"

#include "Components/Camera.h"

Engine* Engine::instance = nullptr;

Engine::Engine(sf::RenderWindow* _window, unsigned _FPS):
	clock(), acu(0.0f), frames(0),
    pause(false)
{
    instance = this;

    _window->setFramerateLimit(_FPS);

    Input::init(_window);
    Random::init();

    GraphicEngine::create();
    PhysicEngine::create();
    ScriptEngine::create();

    Material::base = new ModelMaterial("baseMaterial");
    PhysicMaterial::base = new PhysicMaterial("baseMaterial");

    if (!font.loadFromFile("Resources/Calibri.ttf"))
        Error::add(WARNING, "Engine(): Cannot load font: Resources/Calibri.ttf");
    else
    {
        text.setString("Bonjour");
        text.setFont(font);
        text.setFillColor(sf::Color::Black);
        text.setCharacterSize(20);
        text.setPosition(8, 0);
    }
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

    float loadTime = clock.restart().asSeconds();

    std::cout << "Loaded in " << loadTime << " seconds" << std::endl;
}

bool Engine::update()
{
    Component::deltaTime = clock.restart().asSeconds();


    if (Input::event->type == sf::Event::Resized)
        GraphicEngine::get()->updateCameraViewPort();

    /// Update events
        Input::update();


    if (pause)
        return false;


    sf::Clock timer;
    float pTime, dTime;


    /// Update scripts
        ScriptEngine::get()->start();
        ScriptEngine::get()->update();

    /// Step physic simulation
        timer.restart();
        PhysicEngine::get()->simulate();
        pTime = timer.getElapsedTime().asSeconds();

    /// Re-update scripts
        ScriptEngine::get()->lateUpdate();

    /// Render scene
        timer.restart();
        GraphicEngine::get()->render();
        dTime = timer.getElapsedTime().asSeconds();



    acu += Component::deltaTime;
    if (acu >= 1.0f)
    {
        text.setString("FPS: " + toString<unsigned>(frames) +
                       "\nDraw time:   " + toString<float>(1000.0f * dTime) + " ms" +
                       "\nPhysic time: " + toString<float>(1000.0f * pTime) + " ms");

        frames = 0;
        acu = 0.0f;
    }
    else
        frames++;

    Input::window->pushGLStates();
        Program::current = 0;
        Input::window->draw(text);
    Input::window->popGLStates();

    return true;
}

void Engine::clear()
{
    Entity::clear();
        std::cout << "Entities" << std::endl;

    Mesh::clear();
        std::cout << "Meshes" << std::endl;

    Program::clear();
        std::cout << "Programs" << std::endl;
    Texture::clear();
        std::cout << "Textures" << std::endl;
    Material::clear();
        std::cout << "Materials" << std::endl;
    PhysicMaterial::clear();
        std::cout << "Physic materials" << std::endl;

    GraphicEngine::get()->clear();
    PhysicEngine::get()->clear();
    ScriptEngine::get()->clear();

    Camera::main = nullptr;


    if (Component::instances != 0)
    {
        std::string text = "One or more component have not been deleted ("+toString(Component::instances)+")";
        MessageBox(nullptr, text.c_str(),
                            "MinGE: closing error", MB_ICONWARNING);

        Component::instances = 0;

//        exit(EXIT_FAILURE);
    }
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
