#include <chrono>
#include <SDL2/SDL.h>

#include "Profiler/profiler.h"
#include "UI/UI.h"

#include "Render/RenderEngine.h"
#include "Render/CommandBuffer.h"

#include "Render/Debug.h"
#include "Render/Shaders/Shader.h"
#include "Render/Shaders/Material.inl"
#include "Render/Textures/Texture.h"

#include "IO/Input.h"

Material RenderEngine::default_material;
static std::vector<cmd_buffer_t*> buffers;

void RenderEngine::init()
{
	GL::init();
	Shader::setup_builtins();
	Debug::init();
	UI::init();

	cmd_buffer_t::init();

#ifdef PROFILE
	MicroProfileDrawInitGL();
	MicroProfileGpuInitGL();
#endif

	default_material = Material::create(Shader::load("asset:shader/standard"));
	default_material.set("color", vec3(0.8f));
	default_material.set("metallic", 0.0f);
	default_material.set("roughness", 0.5f);
}

void RenderEngine::destroy()
{
	default_material.destroy();

	cmd_buffer_t::destroy();

	UI::destroy();
	Debug::destroy();

	Shader::clear();
	Texture::clear();
	Mesh::clear();
	Material::clear();

	GL::destroy();
}


void RenderEngine::start_frame()
{
	/// Profiler
	ivec2 mouse_pos = Input::mouse_position(false);
	MicroProfileModKey(Input::key_down(Key::LeftShift));
	MicroProfileMousePosition(mouse_pos.x, mouse_pos.y, -Input::wheel_scroll());
	MicroProfileMouseButton(Input::button_down(Button::Left), Input::button_down(Button::Right));

	/// UI
	UI::frame();
}

void RenderEngine::flush()
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "flush");

	material_t::bound = nullptr;

	for (auto *cmd : buffers)
		cmd->flush();

	Debug::flush();
	UI::flush();

#ifdef PROFILE
	{
		MicroProfileFlip();

		MICROPROFILE_SCOPEI("RENDER_ENGINE", "profiler");
		MICROPROFILE_SCOPEGPUI("profiler", -1);

		ivec2 size = Input::window_size();
		MicroProfileBeginDraw(size.x, size.y, 1.f);

		MicroProfileDraw(size.x, size.y);

		MicroProfileEndDraw();
	}
#endif

	{
		MICROPROFILE_SCOPEI("RENDER_ENGINE", "swap");

		SDL_GL_SwapWindow(Input::window());
	}
}

void RenderEngine::add_buffer(cmd_buffer_t *buffer)
{
	buffers.emplace_back(buffer);
}

void RenderEngine::remove_buffer(cmd_buffer_t *buffer)
{
	for (auto i = buffers.begin(); i != buffers.end(); ++i)
	{
		if (*i == buffer)
		{
			buffers.erase(i);
			return;
		}
	}
}
