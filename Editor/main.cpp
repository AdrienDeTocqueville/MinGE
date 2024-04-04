#include <doge.h>
#include <SDL2/SDL.h>

#include "Editor.h"

int main(int argc, char **argv)
{
	/// Create window
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = Input::create_window_centered("doge Editor", vec2(0.5f, 0.5f),
		SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE, 0);

	// Register object types & importers
	AssetDatabase::register_object_type(Scene::type);
	AssetDatabase::register_object_type(Entity::type);
	AssetDatabase::register_object_type(Texture::type);
	AssetDatabase::register_object_type(Mesh::type);
	AssetDatabase::register_object_type(Shader::type);
	AssetDatabase::register_object_type(Material::type);

	for (const char *format : { "JPG", "PNG", "TGA", "BMP", "PSD", "GIF", "HDR", "PIC" })
		AssetDatabase::register_importer(format, Texture::importer);
	AssetDatabase::register_importer("shader", Shader::importer);
	AssetDatabase::register_importer("gltf", Scene::importer);

	/// Init engine
	Engine::init(argc, argv, window);
	Engine::register_system_type(TransformSystem::type);
	Engine::register_system_type(GraphicsSystem::type);
	Engine::register_system_type(PostProcessingSystem::type);

	Texture tex0 = AssetDatabase::get("data/Textures/0.png");
	Texture texWood = AssetDatabase::get("data/Textures/wood.png");

	Scene simple_scene = AssetDatabase::get("data/Scenes/simple.gltf");


	/// Main loop
	while (!Input::window_closed())
	{
		if (Input::key_pressed(Key::F8))
			MicroProfileToggleDisplayMode();
		if (Input::key_pressed(Key::F7))
			MicroProfileTogglePause();

		if (!ImGui::GetIO().WantTextInput && Input::key_down(Key::LeftControl))
		{
			if (Input::key_down(Key::C))
				Input::close_window();
		}

		Editor::frame();
		Engine::frame();
	}

	Engine::destroy();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
