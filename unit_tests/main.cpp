#include <MinGE.h>

#include <iostream>

#include "tests.h"

SDL_Window* window;

void open_window()
{
	// OpenGL 4
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	window = SDL_CreateWindow("MinGE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1440, 810, window_flags);
}

int main(int, char**)
{
	std::cout << "  -- MinGE --" << std::endl;

	/// Create window
	open_window();

	/// Init engine
	Engine::init(window);

	test_structures();
	test_transforms();
	//test_systems();
	benchmark_transforms(50);

	Engine::destroy();

	SDL_DestroyWindow(window);
	SDL_Quit();

	printf("\n\nAll tests passed.\nPress enter to quit...");
	std::cin.get();
	return 0;
}
