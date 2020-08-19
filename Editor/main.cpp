#include <doge.h>
#include <SDL2/SDL.h>

#include "Editor.h"

int main(int argc, char **argv)
{
	/// Create window
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = Input::create_window_maximized("doge Editor",
		SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE, 1);

	/// Init engine
	Engine::init(window);

	const char *scene = "pbr";
	if (argc == 2) scene = argv[1];

	char scene_path[255];
	stbsp_snprintf(scene_path, 255, "Assets/Scenes/%s.ge", scene);
	Editor::open_scene(scene_path);

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
