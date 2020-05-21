#include <MinGE.h>
#include <SDL.h>

#include <iostream>

#include "Editor.h"

SDL_Window* window;
SDL_GLContext gl_context;

void open_window(int monitor)
{
	int pos = SDL_WINDOWPOS_CENTERED_DISPLAY(monitor);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI |
		SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);

	// OpenGL 4
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	window = SDL_CreateWindow("MinGE", pos, pos, 1440, 810, window_flags);

	//SDL_SetWindowPosition(window, -1920 + (1920-1440)/2, 350);
}

int main(int, char**)
{
	std::cout << "  -- MinGE --" << std::endl;

	/// Create window
	open_window(1);

	/// Init engine
	Engine::init(window);
	Editor::init();

	/// Init systems
	auto transforms = new(Engine::alloc_system("TransformSystem")) TransformSystem();
	auto graphics = new(Engine::alloc_system("GraphicsSystem")) GraphicsSystem(transforms);

	// Open assets
	Mesh cube = Mesh::import("asset:mesh/cube?x=1&y=3&z=3");
	Mesh sphere = Mesh::import("asset:mesh/sphere?radius=3");

	Texture::import("asset://Assets/Textures/0.png?format=srgb");
	Texture::import("asset://Assets/Textures/white.png");

	//Texture::import("asset://Assets/Textures/Iron/albedo.png?format=srgb");
	//Texture::import("asset://Assets/Textures/Iron/metallic.png");
	//Texture::import("asset://Assets/Textures/Iron/roughness.png");

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
