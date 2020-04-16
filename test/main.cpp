#include <MinGE.h>
#include <SFML/Graphics/RenderWindow.hpp>

#include <iostream>

#include "tests.h"

const auto desktop = sf::VideoMode::getDesktopMode();

//#ifdef DEBUG
const auto video_mode = sf::VideoMode(2*desktop.width/3, 2*desktop.height/3);
const auto style = sf::Style::Default;
//#else
//const auto video_mode = desktop;
//const auto style = sf::Style::Fullscreen;
//#endif

int main()
{
	std::cout << "  -- MinGE --" << std::endl;

	/// Create window
	sf::RenderWindow window(video_mode, "MinGE", style, sf::ContextSettings(24, 0, 0, 4, 3));
	window.setPosition(sf::Vector2i(desktop.width - video_mode.width, desktop.height - video_mode.height) / 2);

	/// Init engine
	Engine::init(window, 30);

	//benchmark(50);
	//test_transforms();
	test_systems();

	/// Init systems
	auto transforms = (TransformSystem*)Engine::create_system("TransformSystem", NULL);
	//auto renderers = (RenderSystem*)Engine::create_system("RenderSystem", {transforms});

	// Open assets
	Mesh mesh = Mesh::import("asset:mesh/cube?x=3&y=2&z=1");
//	Texture texture = Texture::import("asset://Assets/level1/floor.png?srgb=1");

	/// Create entities
	Entity e = Entity::create();
	transforms->add(e, vec3(0, 0, 0));
	//renderers->add(e, mesh);

	/// Main loop
	while (!Input::window_closed())
	{
		Engine::start_frame();

		Engine::end_frame();
		window.display();
	}

	Engine::destroy();

	return 0;
}
