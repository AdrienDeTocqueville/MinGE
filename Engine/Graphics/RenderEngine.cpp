#include "Profiler/profiler.h"
#include "UI/ui.h"

#include "Graphics/RenderEngine.h"
#include "Graphics/CommandBuffer.h"

#include "Graphics/Debug.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Shaders/Material.inl"
#include "Graphics/Textures/Texture.h"

#include "IO/Input.h"


static std::vector<cmd_buffer_t> buffers;

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

	// TODO: destroy it
	Material mat = Material::create(Shader::standard());
	mat.set("color", vec3(0.8f));
	mat.set("metallic", 0.0f);
	mat.set("roughness", 0.5f);
}

void RenderEngine::destroy()
{
	Debug::destroy();

	Shader::clear();
	Texture::clear();
	Mesh::clear();
	Material::clear();
}


void RenderEngine::start_frame()
{
	/// Profiler
	ivec2 mouse_pos = Input::mouse_position(false);
	MicroProfileMouseButton(Input::button_pressed(sf::Mouse::Left), Input::button_pressed(sf::Mouse::Right));
	MicroProfileMousePosition(mouse_pos.x, mouse_pos.y, -Input::wheel_scroll());
	MicroProfileModKey(Input::key_down(sf::Keyboard::LShift));

	/// UI
	UI::send_inputs();
}

void RenderEngine::flush()
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "flush");

	for (auto &cmd : buffers)
	{
		MICROPROFILE_SCOPEI("RENDER_ENGINE", "cmd_buffer");
		MICROPROFILE_SCOPEGPUI("cmd_buffer", -1);
		cmd.flush();
	}

	// TODO: this will draw on the last camera
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
}

uint32_t RenderEngine::create_cmd_buffer()
{
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
}
