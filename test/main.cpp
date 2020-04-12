#include <MinGE.h>
#include <SFML/Graphics/RenderWindow.hpp>

#include <iostream>

const auto desktop = sf::VideoMode::getDesktopMode();

//#ifdef DEBUG
const auto video_mode = sf::VideoMode(2*desktop.width/3, 2*desktop.height/3);
const auto style = sf::Style::Default;
//#else
//const auto video_mode = desktop;
//const auto style = sf::Style::Fullscreen;
//#endif

void benchmark(int iterations)
{
	long time = 0;
	for (int i = 0; i < iterations; i++)
	{
		TransformSystem transforms;
		Entity entities[1000];

		Time::Chrono timer;
		for (int j = 0; j < 1000; j++)
		{
			entities[j] = Entity::create();
			transforms.add(entities[j], vec3(9, 1, 0));

			for (int k = 0; k < 5; k++)
			{
				Entity e = Entity::create();
				transforms.add_child(entities[j], e);

				for (int l = 0; l < 4; l++)
				{
					Entity c = Entity::create();
					transforms.add_child(e, c);
				}
			}

			transforms.get(entities[j]).set_position(vec3(0.0f));
		}
		time += timer.time();
	}
	std::cout << time / iterations << "\n";
}

int main()
{
	std::cout << "  -- MinGE --" << std::endl;


	/// Create window
	sf::RenderWindow window(video_mode, "MinGE", style, sf::ContextSettings(24, 0, 0, 4, 3));
	window.setPosition(sf::Vector2i(desktop.width - video_mode.width, desktop.height - video_mode.height) / 2);

	/// Init engine
	Engine::init(window, 30);

	//benchmark(50); transform system type is not registered

	/// Init systems
	Engine::register_system_type(TransformSystem::type);
	//Engine::register_system_type(SYSTEM_TYPE(RenderSystem, 1));
	auto transforms = (TransformSystem*)Engine::create_system("TransformSystem", NULL);
	//auto renderers = (RenderSystem*)Engine::create_system("RenderSystem", {transforms});

	// Open assets
//	Mesh mesh = Assets::load_mesh("asset:mesh/cube?width=3&height=2");
//	Texture texture = Assets::load_texture("asset:///Assets/level1/floor.png?srgb=1");

	/// Create entities
	Entity e = Entity::create();
	transforms->add(e, vec3(0, 0, 0));
	//renderers->add(e, mesh);

	/// Main loop
	while (Input::isOpen())
	{
		Engine::start_frame();

		Engine::end_frame();
		window.display();
	}

	// Creates global asset 'level'
	// Creates backup file 'Assets/level.ge.old' if necessary
	// Create or clear file 'Assets/level.ge' with Engine serialized data
	Scene::system_ref_t systems[] = {
		Scene::system_ref_t{"world", transforms},
		//{"RenderSystem", "world_renderers", renderers},
	};
	Scene level(ARRAY_LEN(systems), systems);
	level.save("Assets/", "level", false);

	/*
	// Opens file 'Assets/level.ge'
	Scene level("Scenes/", "level")
	TransformSystem *transforms = level.get_system("world");
	*/

	Engine::destroy();

	return 0;
}
