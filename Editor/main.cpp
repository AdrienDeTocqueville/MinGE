#include <MinGE.h>
#include <SDL2/SDL.h>

#include <iostream>

#include "Editor.h"

// temp for blinn phong
#include "Render/Shader/Shader.h"
#include "Render/Shader/Material.inl"

int main(int, char**)
{
	std::cout << "  -- MinGE --" << std::endl;

	/// Create window
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = Input::create_window_maximized("MinGE",
		SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE, 1);

	/// Init engine
	Engine::init(window);
	Editor::init();

	Editor::open_scene("Assets/Scenes/gamma.ge");

	auto m = Material::get(4);
	m.set("tiling", vec2(5.0f, 5.0f));
	m.set("color_map", Texture::get(7));

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
