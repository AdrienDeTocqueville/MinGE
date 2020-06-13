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

	Texture::load("asset://Textures/0.png?format=srgb");
	Texture::load("asset://Textures/white.png");

	Editor::open_scene("Assets/tests/graphics_test.ge");

	/// Main loop
	while (!Input::window_closed())
	{
		if (Input::key_pressed(Key::F8))
			MicroProfileToggleDisplayMode();
		if (Input::key_pressed(Key::F7))
			MicroProfileTogglePause();

		Editor::frame();
		Engine::frame();
	}

	Engine::destroy();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
