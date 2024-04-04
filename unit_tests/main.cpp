#include <doge.h>

#include "tests.h"

int main(int argc, char **argv)
{
	/// Create window
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = Input::create_window_centered("unit tests", vec2(0.66f),
		SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE, 1);

	/// Init engine
	Engine::init(argc, argv, window);
	printf("Engine initialized\n");

	LAUNCH(entity);
	LAUNCH(scene);
	LAUNCH(structures);
	LAUNCH(transforms);
	//LAUNCH(systems);
	BENCH(transforms_creation, 50);
	BENCH(transforms_access, 50);

	Engine::destroy();

	SDL_DestroyWindow(window);
	SDL_Quit();

	printf("\n\nAll tests passed.\n");
#ifdef _WIN32
	printf("Press enter to quit..."); std::cin.get();
#endif
	return 0;
}
