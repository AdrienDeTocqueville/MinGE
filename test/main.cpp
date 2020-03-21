#include <MinGE.h>
#include <SFML/Graphics/RenderWindow.hpp>

const auto desktop = sf::VideoMode::getDesktopMode();

#ifdef DEBUG
const auto video_mode = sf::VideoMode(2*desktop.width/3, 2*desktop.height/3);
const auto style = sf::Style::Default;
#else
const auto video_mode = desktop;
const auto style = sf::Style::Fullscreen;
#endif


int main()
{
	std::cout << "  -- MinGE --" << std::endl;


	/// Create window
	sf::RenderWindow window(video_mode, "MinGE", style, sf::ContextSettings(24, 0, 0, 4, 3));
	window.setPosition(sf::Vector2i(desktop.width - video_mode.width, desktop.height - video_mode.height) / 2);


	/// Init engine
	Engine::init(&window, 30);

	/// Init systems
	TransformSystem transforms;
	//GraphicsSystem graphics(transforms);

	/// Create entities
	Entity entity = Entity::create();
	transforms.add(entity, vec3(10, 0, 0));

	Entity e2 = Entity::create();
	transforms.add(e2, vec3(9, 1, 0));

	transforms.add_child(entity, e2);

	Transform tr = transforms.get(entity);
	tr.set_position(tr.position() + vec3(1, 0, 0));
	write(transforms.get(e2).world_matrix());

	/// Main loop
	while (Input::isOpen())
	{
		Engine::start_frame();

		//graphics.draw();

		Engine::end_frame();
		window.display();
	}

	/*
	// Creates global asset 'level'
	// Creates backup file 'Assets/level.ge.old' if necessary
	// Create or clear file 'Assets/level.ge' with Engine serialized data
	Assets::create("level");
	// Appends to file from global asset 'level'
	transforms.serialize("level");

	// Opens file 'Assets/level.ge'
	Assets::open("level");
	transforms.deserialize("level");
	*/

	Engine::destroy();

	return 0;
}
