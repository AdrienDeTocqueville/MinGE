#include <iostream>
#include <MinGE.h>
#include <SDL2/SDL.h>

#include "CameraControl.h"

#define SERIALIZE

int main(int, char**)
{
	std::cout << "  -- MinGE --" << std::endl;

	/// Create window
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = Input::create_window_centered("MinGE", vec2(0.66f),
		SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE, 1);

	/// Init engine
	Engine::init(window);

	Engine::register_system_type(TransformSystem::type);
	Engine::register_system_type(GraphicsSystem::type);
	Engine::register_system_type(PostProcessingSystem::type);
	Engine::register_system_type(CameraControl::type);

	Engine::register_asset_type(Mesh::type);
	Engine::register_asset_type(Texture::type);

	Texture::load("asset://Textures/0.png?format=srgb8");
	Texture::load("asset://Textures/white.png");

#ifdef SERIALIZE
	/// Init systems
	auto transforms = new(Engine::alloc_system("TransformSystem")) TransformSystem();
	auto graphics = new(Engine::alloc_system("GraphicsSystem")) GraphicsSystem(transforms);
	auto controller = new(Engine::alloc_system("CameraControl")) CameraControl(transforms, graphics);

	// Open assets
	Mesh cube = Mesh::load("asset:mesh/cube?x=1&y=3&z=3");
	Mesh sphere = Mesh::load("asset:mesh/sphere?radius=3");

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
	auto cam = graphics->add_camera(camera_ent);
	transforms->get(camera_ent).look_at(vec3(5,0,0));
	controller->add(camera_ent);

	camera_ent = Entity::create("Camera2");
	transforms->add(camera_ent, vec3(-6,0,0));
	graphics->add_camera(camera_ent, 70.0f, 2.0f, 10.0f);
	controller->add(camera_ent);

	Texture output_color = cam.color_texture();
	auto postproc = new(Engine::alloc_system("PostProcessingSystem")) PostProcessingSystem(graphics, output_color);


	Scene s("transforms", transforms, "graphics", graphics, "post-processing", postproc);
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

#ifdef SERIALIZE
	cube.destroy();
	sphere.destroy();
#else
	s.clear();
#endif

	Engine::destroy();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
