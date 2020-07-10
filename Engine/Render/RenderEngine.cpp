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
static std::vector<cmd_buffer_t> buffers;

GLuint empty_vao;

void RenderEngine::init()
{
	GL::init();
	Shader::setup_builtins();
	Debug::init();
	UI::init();

#ifdef PROFILE
	MicroProfileDrawInitGL();
	MicroProfileGpuInitGL();
#endif

	default_material = Material::create(Shader::load("asset:shader/standard"));
	default_material.set("color", vec3(0.8f));
	default_material.set("metallic", 0.0f);
	default_material.set("roughness", 0.5f);

	empty_vao = GL::GenVertexArray();
}

void RenderEngine::destroy()
{
	GL::DeleteVertexArray(empty_vao);

	default_material.destroy();

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

	for (auto &cmd : buffers)
	{
		MICROPROFILE_SCOPEI("RENDER_ENGINE", "cmd_buffer");
		MICROPROFILE_SCOPEGPUI("cmd_buffer", -1);
		cmd.flush();
	}

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	GL::BindFramebuffer(0);
	GL::Enable(GL::Blend);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

uint32_t RenderEngine::create_cmd_buffer()
{
	for (int i = 0; i < buffers.size(); i++)
	{
		if (buffers[i].buffer == NULL)
		{
			new(&buffers[i]) cmd_buffer_t();
			return i;
		}
	}
	buffers.emplace_back();
	return (uint32_t)buffers.size() - 1;
}

cmd_buffer_t &RenderEngine::get_cmd_buffer(uint32_t i)
{
	return buffers[i];
}

void RenderEngine::destroy_cmd_buffer(uint32_t i)
{
	free(buffers[i].buffer);
	buffers[i].buffer = NULL;
	buffers[i].size = 0;
}
