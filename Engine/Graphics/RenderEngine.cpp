#include "Profiler/profiler.h"

#include "Graphics/RenderEngine.h"
#include "Graphics/CommandBuffer.h"

#include "Graphics/Debug.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Shaders/Material.inl"
#include "Graphics/Textures/Texture.h"


static std::vector<cmd_buffer_t> buffers;

void RenderEngine::init()
{
	GL::init();
	Shader::setup_builtins();
	Debug::init();

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

void RenderEngine::flush()
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "flush");

	for (auto &cmd : buffers)
	{
		MICROPROFILE_SCOPEI("RENDER_ENGINE", "cmd_buffer");
		cmd.flush();
	}

	{ MICROPROFILE_SCOPEI("RENDER_ENGINE", "debug_draw");
	// TODO: this will draw on the last camera
	Debug::flush();
	}
}

uint32_t RenderEngine::create_cmd_buffer()
{
	buffers.emplace_back();
	return buffers.size() - 1;
}

cmd_buffer_t &RenderEngine::get_cmd_buffer(uint32_t i)
{
	return buffers[i];
}

void RenderEngine::destroy_cmd_buffer(uint32_t i)
{
	free(buffers[i].buffer);
}
