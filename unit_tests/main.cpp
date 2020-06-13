#include <iostream>
#include <MinGE.h>

#include "tests.h"

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

	LAUNCH(entity);
	LAUNCH(structures);
	LAUNCH(transforms);
	//LAUNCH(systems);
	BENCH(transforms, 50);

	Engine::destroy();

	SDL_DestroyWindow(window);
	SDL_Quit();

	printf("\n\nAll tests passed.\n");
#ifdef _WIN32
	printf("Press enter to quit..."); std::cin.get();
#endif
	return 0;
}
