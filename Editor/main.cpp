#include <MinGE.h>
#include <SDL2/SDL.h>

#include <iostream>

#include "Editor.h"

int main(int, char**)
{
	std::cout << "  -- MinGE --" << std::endl;

	/// Create window
	int monitor = 1;
	int pos = SDL_WINDOWPOS_CENTERED_DISPLAY(monitor);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI |
		SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);

	SDL_Window *window = SDL_CreateWindow("MinGE", pos, pos, 1440, 810, window_flags);
	//SDL_SetWindowPosition(window, -1920 + (1920-1440)/2, 350);

	/// Init engine
	Engine::init(window);
	Editor::init();

	Texture::load("asset://Assets/Textures/0.png?format=srgb");
	Texture::load("asset://Assets/Textures/white.png");

	//Texture::load("asset://Assets/Textures/Iron/albedo.png?format=srgb");
	//Texture::load("asset://Assets/Textures/Iron/metallic.png");
	//Texture::load("asset://Assets/Textures/Iron/roughness.png");

	Scene s = Scene::load("asset://Assets/tests/graphics_test.ge");
	auto transforms = (TransformSystem*)s.get_system("transforms");
	auto graphics = (GraphicsSystem*)s.get_system("graphics");

	s.save("asset://Assets/tests/editor_scene.ge");

	Entity mesh_ent = Entity::get("Sphere");

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
