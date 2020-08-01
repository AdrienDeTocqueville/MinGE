#include "Profiler/profiler.h"

#include "Render/CommandBuffer.h"
#include "Render/Shader/Shader.inl"
#include "Render/Shader/Material.inl"

static GLuint empty_vao;

static uint32_t GLOBAL, PER_OBJECT;

void cmd_buffer_t::init()
{
	empty_vao = GL::GenVertexArray();

#define INIT_LOCATION(x) x = Shader::get_builtin_location(#x)
	INIT_LOCATION(GLOBAL);
	INIT_LOCATION(PER_OBJECT);
#undef INIT_LOCATION
}

void cmd_buffer_t::destroy()
{
	GL::DeleteVertexArray(empty_vao);
}


void cmd_buffer_t::set_uniform_data(uint32_t buffer, void *data, uint32_t size)
{
	block_data_t cmd = { buffer, size, data };
	store(SetUniformData, cmd);
}

void cmd_buffer_t::set_storage_data(uint32_t buffer, void *data, uint32_t size)
{
	block_data_t cmd = { buffer, size, data };
	store(SetStorageData, cmd);
}

void cmd_buffer_t::setup_camera(ivec2 res, uint32_t buf, uint32_t offset, uint32_t size)
{
	setup_camera_t cmd = { res, buf, offset, size };
	store(SetupCamera, cmd);
}

void cmd_buffer_t::set_framebuffer(uint32_t fbo, vec4 color, bool clear_depth)
{
	set_framebuffer_t cmd = { fbo, clear_depth, color };
	store(SetFramebuffer, cmd);
}

void cmd_buffer_t::draw_batch(submesh_data_t *submeshes, uint32_t *sorted_indices,
	uint32_t per_object, RenderPass::Type pass, uint32_t count)
{
	draw_batch_t cmd = {
		submeshes, sorted_indices,
		per_object, pass, count
	};
	store(DrawBatch, cmd);
}

void cmd_buffer_t::fullscreen_pass(uint32_t fbo, ivec4 viewport, uint32_t material)
{
	fullscreen_pass_t cmd = { fbo, material, viewport };
	store(FullscreenPass, cmd);
}

void cmd_buffer_t::flush()
{
MICROPROFILE_SCOPEI("RENDER_ENGINE", "cmd_buffer_flush");
MICROPROFILE_SCOPEGPUI("cmd_buffer_flush", -1);

uint32_t i = 0;
while (i < size)
{
Type cmd = consume<Type>(i);
switch (cmd)
{
case SetUniformData:
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "SetUniformData");
	MICROPROFILE_SCOPEGPUI("SetUniformData", -1);

	block_data_t &data = consume<block_data_t>(i);

	GL::BindUniformBuffer(data.buffer);
	glCheck(glBufferData(GL_UNIFORM_BUFFER, data.size, data.data, GL_DYNAMIC_DRAW));

	break;
}

case SetStorageData:
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "SetStorageData");
	MICROPROFILE_SCOPEGPUI("SetStorageData", -1);

	block_data_t &data = consume<block_data_t>(i);

	GL::BindStorageBuffer(data.buffer);
	glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, data.size, data.data, GL_DYNAMIC_DRAW));

	break;
}

case DrawBatch:
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "DrawBatch");
	MICROPROFILE_SCOPEGPUI("DrawBatch", -1);

	const unsigned object_size = mem::align(sizeof(mat4) * 2, GL::uniform_offset_alignment);
	draw_batch_t &batch = consume<draw_batch_t>(i);

	for (uint32_t c = 0; c < batch.count; c++)
	{
		uint32_t s = batch.sorted_indices[c];
		submesh_data_t *data = batch.submeshes + s;
		material_t *material = Material::materials.get<0>(data->material);
		if (!material->has_pass(batch.pass))
			continue;

		Shader::set_uniform(PER_OBJECT, batch.per_object, data->renderer * object_size, object_size);

		// 0.84 + 0.91
		material->bind(batch.pass);

		// 4.15 + 4.12
		GL::BindVertexArray(data->submesh.vao);
		glDrawElements(data->submesh.mode, data->submesh.count,
			GL_UNSIGNED_SHORT, (void*)(uint64_t)data->submesh.offset);
	}

	break;
}

case SetFramebuffer:
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "SetFramebuffer");
	MICROPROFILE_SCOPEGPUI("SetFramebuffer", -1);

	set_framebuffer_t &setup = consume<set_framebuffer_t>(i);

	GL::BindFramebuffer(setup.fbo);
	GL::ClearColor(setup.clear_color);
	if (setup.clear_depth)
	{
		GL::DepthMask(true);
		GL::DepthFunc(GL::LessEqual);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
	else
	{
		GL::DepthMask(false);
		GL::DepthFunc(GL::Equal);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	break;
}

case SetupCamera:
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "SetupCamera");
	MICROPROFILE_SCOPEGPUI("SetupCamera", -1);

	setup_camera_t &setup = consume<setup_camera_t>(i);

	GL::Enable(GL::CullFace);
	GL::Enable(GL::DepthTest);
	GL::Enable(GL::ScissorTest);
	GL::Disable(GL::Blend);

	ivec4 viewport = ivec4(0, 0, setup.res.x, setup.res.y);
	GL::Viewport(viewport);
	GL::Scissor(viewport);

	Shader::set_storage(GLOBAL, setup.buf, setup.offset, setup.size);

	break;
}

case FullscreenPass:
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "FullscreenPass");
	MICROPROFILE_SCOPEGPUI("FullscreenPass", -1);

	fullscreen_pass_t &setup = consume<fullscreen_pass_t>(i);

	GL::BindFramebuffer(setup.fbo);

	GL::Enable(GL::DepthTest);
	GL::DepthMask(true);
	GL::DepthFunc(GL::Always);

	GL::Viewport(setup.viewport);
	GL::Scissor(setup.viewport);

	material_t *material = Material::materials.get<0>(setup.material);
	if (!material->has_pass(RenderPass::Forward))
		break;

	material->bind(RenderPass::Forward);

	GL::BindVertexArray(empty_vao);
	glCheck(glDrawArrays(GL_TRIANGLES, 0, 3));

	break;
}
}
}
size = 0;
}
