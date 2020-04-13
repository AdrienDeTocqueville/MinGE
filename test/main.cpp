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

void test_transforms()
{
	auto transforms = (TransformSystem*)Engine::create_system("TransformSystem", NULL);

	/// Create entities
	Entity e1 = Entity::create();
	Entity e2 = Entity::create();
	Entity e3 = Entity::create();
	Entity _e = Entity::create();
	Entity e4 = Entity::create();
	Entity e5 = Entity::create();
	Entity e6 = Entity::create();
	Entity e7 = Entity::create();
	Entity e8 = Entity::create();

	transforms->add(e1, vec3(0, 0, 0));

	transforms->add_child(e1, e2);
	transforms->add_child(e1, e3);
	transforms->add_child(e1, e4, vec3(1, 0, 0));

	transforms->add(_e);

	transforms->add_child(e4, e5, vec3(0, 1, 0));
	transforms->add_child(e4, e6, vec3(1, 0, 0));

	transforms->remove(_e);

	transforms->add_child(e5, e7, vec3(0, 0, 1));
	transforms->add_child(e5, e8, vec3(0, 1, 0));

	transforms->get(e1).set_position(vec3(0.0f));


	Scene::system_ref_t systems[] = {
		Scene::system_ref_t{"world", transforms},
	};
	Scene level(systems, ARRAY_LEN(systems));
	level.save("Assets/", "level", false);
}

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

	/// Init systems
	auto transforms = (TransformSystem*)Engine::create_system("TransformSystem", NULL);
	//auto renderers = (RenderSystem*)Engine::create_system("RenderSystem", {transforms});

	// Open assets
//	Mesh mesh = Mesh::import("asset:mesh/cube?width=3&height=2");
//	Texture texture = Texture::import("asset:///Assets/level1/floor.png?srgb=1");

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

	Engine::destroy();

	return 0;
}
