#include <iostream>
#include <MinGE.h>
#include <SDL2/SDL.h>

#include "CameraControl.h"

#define SERIALIZE

int main(int, char**)
{
	std::cout << "  -- MinGE --" << std::endl;

	/// Create window
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window *window = SDL_CreateWindow("MinGE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1440, 810, window_flags);

	SDL_SetWindowPosition(window, -1920 + (1920-1440)/2, 350);

	/// Init engine
	Engine::init(window);

	Engine::register_system_type(TransformSystem::type);
	Engine::register_system_type(GraphicsSystem::type);
	Engine::register_system_type(CameraControl::type);

	Engine::register_asset_type(Mesh::type);
	Engine::register_asset_type(Texture::type);

	Texture::load("asset://Textures/0.png?format=srgb");
	Texture::load("asset://Textures/white.png");

#ifdef SERIALIZE
	/// Init systems
	auto transforms = new(Engine::alloc_system("TransformSystem")) TransformSystem();
	auto graphics = new(Engine::alloc_system("GraphicsSystem")) GraphicsSystem(transforms);
	auto controller = new(Engine::alloc_system("CameraControl")) CameraControl(transforms, graphics);

	// Open assets
	Mesh cube = Mesh::load("asset:mesh/cube?x=1&y=3&z=3");
	Mesh sphere = Mesh::load("asset:mesh/sphere?radius=3");

	//Texture texture = Texture::load("asset://Textures/0.png?format=srgb");

	/// Create entities
	Entity mesh_ent = Entity::create("Cube");
	transforms->add(mesh_ent, vec3(2, 0, 0), vec3(0,0,PI*0.25f));
	graphics->add_renderer(mesh_ent, cube);

	mesh_ent = Entity::create("Sphere");
	transforms->add(mesh_ent, vec3(10, 0, 0));
	graphics->add_renderer(mesh_ent, sphere);

	Entity light_ent = Entity::create("Light");
	transforms->add(light_ent, vec3(5, 0, 8));
	graphics->add_point_light(light_ent);

	Entity camera_ent = Entity::create("MainCamera");
	transforms->add(camera_ent, vec3(5, 13, 10));
	graphics->add_camera(camera_ent);
	transforms->get(camera_ent).look_at(vec3(5,0,0));
	controller->add(camera_ent);

	camera_ent = Entity::create("Camera2");
	transforms->add(camera_ent, vec3(-6,0,0));
	graphics->add_camera(camera_ent, 70.0f, 2.0f, 8.0f);
	controller->add(camera_ent);

	Scene s("transforms", transforms, "graphics", graphics);
	s.save("Assets/tests/graphics_test.ge");

#else
	Scene s("Assets/tests/graphics_test.ge");
	auto transforms = (TransformSystem*)s.get_system("transforms");
	auto graphics = (GraphicsSystem*)s.get_system("graphics");

	auto controller = new(Engine::alloc_system("CameraControl")) CameraControl(transforms, graphics);

	controller->add(Entity::get("MainCamera"));
	controller->add(Entity::get("Camera2"));

	Entity mesh_ent = Entity::get("Sphere");
#endif

	/// Main loop
	while (!Input::window_closed())
	{
		if (Input::key_pressed(Key::F8))
			MicroProfileToggleDisplayMode();
		if (Input::key_pressed(Key::F7))
			MicroProfileTogglePause();

		transforms->get(mesh_ent).rotate(vec3(0,0,1), Time::delta_time);

		Engine::frame();
	}

	Engine::destroy();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
