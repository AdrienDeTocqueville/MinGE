#include <MinGE.h>

#include "physic/physic.h"
#include "bvh/bvh.h"
#include "materials/materials.h"


const auto desktop = sf::VideoMode::getDesktopMode();

#ifdef DEBUG
const auto video_mode = sf::VideoMode(2*desktop.width/3, 2*desktop.height/3);
const auto style = sf::Style::Default;
#else
const auto video_mode = desktop;
const auto style = sf::Style::Fullscreen;
#endif

int scene = 2;
std::vector<void (*)()> setups = {test_physic, test_bvh, test_materials};
std::vector<std::string> names = {"physic", "bvh", "materials"};

void start_scene(Engine *engine, int _scene)
{
	// Light source
	//MaterialRef bright = Material::getDefault();
	//	bright->set("ambient", vec3(2.0f));
	//	bright->set("diffuse", vec3(0.0f));
	//	bright->set("specular", vec3(0.0f));
	//	bright->set("exponent", 0.0f);
	//	bright->set("mainTexture", Texture::get("Textures/white.png");

	Entity::create("Light", false, vec3(-5, 0, 0))
		//->insert<Graphic>(Mesh::createSphere(ALLFLAGS, 0.25f), {bright})
		->insert<Light>(GE_POINT_LIGHT, vec3(0.0f), vec3(0.9f), 1.0f);

	scene = _scene % setups.size();
	Input::getWindow()->setTitle("MinGE (test " + names[scene] + ")");
	setups[scene]();
	engine->start();
}

int main()
{
	std::cout << "  -- MinGE --" << std::endl;


	/// Create window
		sf::RenderWindow window(video_mode, "MinGE test suite", style, sf::ContextSettings(24, 0, 0, 4, 3));
		window.setPosition(sf::Vector2i(desktop.width - video_mode.width, desktop.height - video_mode.height) / 2);


	/// Create engine
		Engine* engine = new Engine(&window, 60);
		std::cout << "Seed: " << Random::getSeed() << std::endl;


	/// Init scene
		start_scene(engine, scene);

	/// Main loop
		while ( Input::isOpen() )
		{
			/// Handle events
				#ifdef DRAWAABB
				if (Input::getKeyReleased(sf::Keyboard::F1))
					AABB::drawAABBs = !AABB::drawAABBs;
				#endif

				if (Input::getKeyReleased(sf::Keyboard::F2))
					GraphicEngine::get()->toggleWireframe();

				if (Input::getKeyReleased(sf::Keyboard::F3))
					PhysicEngine::get()->setGravity();



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

	#if defined(DEBUG) && defined(_WIN32)
		sf::sleep(sf::seconds(1.0f));
	#endif // DEBUG

	return 0;
}
