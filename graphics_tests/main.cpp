#include <MinGE.h>
#include <SFML/Graphics/RenderWindow.hpp>

#include <iostream>

#include "CameraControl.h"

const auto desktop = sf::VideoMode::getDesktopMode();

//#ifdef DEBUG
//const auto video_mode = sf::VideoMode(2*desktop.width/3, 2*desktop.height/3);
const auto video_mode = sf::VideoMode(1440, 810);
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
	window.setPosition(sf::Vector2i(-1920 + (1920-video_mode.width)/2, 350));

	/// Init engine
	Engine::init(window, 30);

	Engine::register_system_type(CameraControl::type);

	/// Init systems
	auto transforms = new(Engine::alloc_system("TransformSystem")) TransformSystem();
	auto graphics = new(Engine::alloc_system("GraphicsSystem")) GraphicsSystem(transforms);
	auto controller = new(Engine::alloc_system("CameraControl")) CameraControl(transforms, graphics);

	// Open assets
	Mesh cube = Mesh::import("asset:mesh/cube?x=1&y=3&z=3");
	Mesh sphere = Mesh::import("asset:mesh/sphere?radius=3");

	//Texture texture = Texture::import("asset://Assets/Textures/0.png?format=srgb");

	/// Create entities
	Entity mesh_ent = Entity::create();
	transforms->add(mesh_ent, vec3(2, 0, 0), vec3(0,0,PI*0.25f));
	graphics->add_renderer(mesh_ent, cube);

	mesh_ent = Entity::create();
	transforms->add(mesh_ent, vec3(10, 0, 0));
	graphics->add_renderer(mesh_ent, sphere);

	Entity light_ent = Entity::create();
	transforms->add(light_ent, vec3(5, 0, 8));
	graphics->add_point_light(light_ent);

	Entity camera_ent = Entity::create();
	transforms->add(camera_ent, vec3(5, 13, 10));
	graphics->add_camera(camera_ent);
	transforms->get(camera_ent).look_at(vec3(5,0,0));
	controller->add(camera_ent);

	camera_ent = Entity::create();
	transforms->add(camera_ent, vec3(-6,0,0));
	graphics->add_camera(camera_ent, 70.0f, 2.0f, 8.0f);
	controller->add(camera_ent);

	/// Main loop
	while (!Input::window_closed())
	{
		if (Input::key_pressed(sf::Keyboard::F8))
			MicroProfileToggleDisplayMode();
		if (Input::key_pressed(sf::Keyboard::F7))
			MicroProfileTogglePause();

		Engine::update();
		transforms->get(mesh_ent).rotate(vec3(0,0,1), Time::delta_time);
		window.display();
	}

	Engine::destroy();

	return 0;
}
