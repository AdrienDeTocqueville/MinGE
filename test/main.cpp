#include <MinGE.h>
#include <SFML/Graphics/RenderWindow.hpp>

#include <iostream>

#include "tests.h"
#include "CameraControl.h"

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

	//benchmark(50);
	//test_transforms();
	//test_systems();
	//test_array_list();

	Engine::register_system_type(CameraControl::type);

	/// Init systems
	auto transforms = new(Engine::alloc_system("TransformSystem", NULL)) TransformSystem();
	auto controller = new(Engine::alloc_system("CameraControl", (const void**)&transforms, 1)) CameraControl(transforms);
	auto graphics = new(Engine::alloc_system("GraphicsSystem", (const void**)&controller, 1)) GraphicsSystem(transforms);

	// Open assets
	Mesh mesh = Mesh::import("asset:mesh/cube?x=1&y=3&z=3");
	//Texture texture = Texture::import("asset://Assets/level1/floor.png?format=srgb");

	/// Create entities
	Entity mesh_ent = Entity::create();
	transforms->add(mesh_ent, vec3(2, 0, 0), vec3(0,0,PI*0.25f));
	graphics->add_renderer(mesh_ent, mesh);

	mesh_ent = Entity::create();
	transforms->add(mesh_ent, vec3(10, 0, 0));
	graphics->add_renderer(mesh_ent, mesh);

	Entity camera_ent = Entity::create();
	transforms->add(camera_ent, vec3(5, 10, 8));
	graphics->add_camera(camera_ent);
	controller->add(camera_ent);

	camera_ent = Entity::create();
	transforms->add(camera_ent, vec3(-2,0,0));
	graphics->add_camera(camera_ent, 70.0f, 1.0f, 5.0f);

	/// Main loop
	while (!Input::window_closed())
	{
		Engine::start_frame();
		Engine::end_frame();
		transforms->get(mesh_ent).rotate(vec3(0,0,1), Time::delta_time);
		window.display();
	}

	Engine::destroy();

	return 0;
}
