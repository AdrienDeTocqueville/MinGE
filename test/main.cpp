#include <MinGE.h>

#include "physic/physic.h"
#include "bvh/bvh.h"


#define FULL_SCREEN sf::VideoMode::getDesktopMode()

#ifdef DEBUG
	#define VIDEOMODE sf::VideoMode(2*FULL_SCREEN.width/3, 2*FULL_SCREEN.height/3)
	#define STYLE sf::Style::Default
#else
	#define VIDEOMODE FULL_SCREEN
	#define STYLE sf::Style::Fullscreen
#endif

int scene = 0;
std::vector<void (*)()> setups = {test_physic, test_bvh};

void start_scene(Engine *engine, int _scene)
{
	Material::base = new ModelMaterial("baseMaterial", "Textures/0.png");
	PhysicMaterial::base = new PhysicMaterial("baseMaterial");

	// Light source
	ModelMaterial* bright = new ModelMaterial("bright");
		bright->ambient = vec3(10.0f/4.0f);
		bright->diffuse = vec3(0.0f);
		bright->specular = vec3(0.0f);
		bright->texture = Texture::get("Textures/white.png");

	Entity::create("Light", false, vec3(5, 2, 4))
		->insert<Graphic>(Mesh::createSphere(bright, ALLFLAGS, 0.25f))
		->insert<Light>(GE_POINT_LIGHT, vec3(0.0f), vec3(0.9f), 1.0f, 1, 0.01, 0);

	scene = _scene % setups.size();
	setups[scene]();
	engine->start();
}

int main()
{
	std::cout << "  -- MinGE --" << std::endl;


	/// Create window
		sf::RenderWindow window(VIDEOMODE, "Gyroscope", STYLE, sf::ContextSettings(24, 0, 0, 4, 3));


	/// Create engine
		Engine* engine = new Engine(&window, 60);
		std::cout << "Seed: " << Random::getSeed() << std::endl;


	/// Init scene
		start_scene(engine, 0);

	/// Main loop
		while ( Input::isOpen() )
		{
			/// Handle events
				if (Input::getKeyReleased(sf::Keyboard::F1))
					AABB::drawAABBs = !AABB::drawAABBs;

				if (Input::getKeyReleased(sf::Keyboard::F2))
					PhysicEngine::get()->setGravity();

				if (Input::getKeyReleased(sf::Keyboard::F3))
					GraphicEngine::get()->toggleWireframe();



				if (Input::getKeyReleased(sf::Keyboard::Escape))
					engine->setPause(true), Input::setCursorMode(CursorMode::Free);

				if (Input::getMousePressed(sf::Mouse::Left) && Input::hasFocus())
					engine->setPause(false), Input::setCursorMode(CursorMode::Capture);

				if (Input::getKeyReleased(sf::Keyboard::Tab))
				{
					engine->clear();
					start_scene(engine, scene + 1);
				}

			/// Render
				if (engine->update())
					window.display();
		}

		std::cout << '\n' << '\n' << std::endl;

	/// Delete resources
		delete engine;

	#ifdef DEBUG
		sf::sleep(sf::seconds(1.0f));
	#endif // DEBUG

	return 0;
}
