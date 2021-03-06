#include <MinGE.h>

#include "physic/physic.h"
#include "bvh/bvh.h"
#include "materials/materials.h"
#include "animations/animations.h"
#include "sky/sky.h"
#include "scene/scene.h"


const auto desktop = sf::VideoMode::getDesktopMode();

#ifdef DEBUG
const auto video_mode = sf::VideoMode(2*desktop.width/3, 2*desktop.height/3);
const auto style = sf::Style::Default;
#else
const auto video_mode = desktop;
const auto style = sf::Style::Fullscreen;
#endif

int scene = 5;
std::vector<void (*)()> setups = {
	test_physic,	// 0
	test_bvh,	// 1
	test_materials,	// 2
	test_animations,// 3
	test_sky,	// 4
	test_scene	// 5
};
std::vector<std::string> names = {"physic", "bvh", "materials", "animations", "sky", "scene"};

void load_scene(Engine *engine)
{
	if (scene < 0) scene = setups.size()-1;
	if (scene >= (int)setups.size()) scene = 0;

	auto sun = Entity::create("Light", false, vec3(0))
		->insert<Light>(Light::Directional, vec3(0.8f), false);
	sun->find<Transform>()->setRotation(glm::rotation(vec3(0, 0, 1), vec3(-0.23171, 0.91854, 0.32032)));

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
	Engine* engine = new Engine(&window, 30);
	std::cout << "Seed: " << Random::getSeed() << std::endl;

	/// Init scene
	load_scene(engine);

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


		if (Input::getKeyReleased(sf::Keyboard::Tab))
		{
			engine->clear();
			if (Input::getKeyDown(sf::Keyboard::LShift))
				scene--;
			else
				scene++;
			load_scene(engine);
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
