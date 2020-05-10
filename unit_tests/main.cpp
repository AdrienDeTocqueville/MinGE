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
	//window.setPosition(sf::Vector2i(desktop.width - video_mode.width, desktop.height - video_mode.height) / 2);

	/// Init engine
	Engine::init(window, 30);

	test_structures();
	test_transforms();
	//test_systems();
	benchmark_transforms(50);

	Engine::destroy();
	window.close();

	printf("\n\nAll tests passed.\nPress enter to quit...");
	std::cin.get();
	return 0;
}
