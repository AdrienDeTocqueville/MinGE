#include "Profiler/profiler.h"

#include "Render/CommandBuffer.h"
#include "Render/Shader/Shader.inl"
#include "Render/Shader/Material.inl"

static GLuint empty_vao;

static size_t MATRIX_M, MATRIX_N;
static size_t MATRIX_VP, VIEW_POS;

void cmd_buffer_t::init()
{
	empty_vao = GL::GenVertexArray();

#define INIT_LOCATION(x) x = Shader::get_builtin_location(#x)
	INIT_LOCATION(MATRIX_M);
	INIT_LOCATION(MATRIX_N);
	INIT_LOCATION(MATRIX_VP);
	INIT_LOCATION(VIEW_POS);
#undef INIT_LOCATION
}

void cmd_buffer_t::destroy()
{
	GL::DeleteVertexArray(empty_vao);

}

void cmd_buffer_t::setup_camera(camera_data_t *camera)
{
	store(SetupCamera, camera);
}

void cmd_buffer_t::set_framebuffer(uint32_t fbo, vec4 color, bool clear_depth)
{
	set_framebuffer_t cmd = { fbo, clear_depth, color };
	store(SetFramebuffer, cmd);
}

void cmd_buffer_t::draw_batch(submesh_data_t *submeshes, mat4 *matrices, uint32_t *sorted_indices,
		RenderPass::Type pass, uint32_t count)
{
	draw_batch_t cmd = {
		submeshes, matrices, sorted_indices,
		pass, count
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
case DrawBatch:
{
	MICROPROFILE_SCOPEI("RENDER_ENGINE", "DrawBatch");
	MICROPROFILE_SCOPEGPUI("DrawBatch", -1);

	draw_batch_t &batch = consume<draw_batch_t>(i);

	for (uint32_t c = 0; c < batch.count; c++)
	{
		uint32_t s = batch.sorted_indices[c];
		submesh_data_t *data = batch.submeshes + s;
		material_t *material = Material::materials.get<0>(data->material);
		if (!material->has_pass(batch.pass))
			continue;

		// 1.79 + 1.62
		Shader::set_builtin(MATRIX_M, batch.matrices[data->renderer]);
		Shader::set_builtin(MATRIX_N, batch.matrices[data->renderer]);

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

	camera_data_t *&camera = consume<camera_data_t*>(i);

	GL::Enable(GL::CullFace);
	GL::Enable(GL::DepthTest);
	GL::Enable(GL::ScissorTest);
	GL::Disable(GL::Blend);

	Shader::set_builtin(MATRIX_VP, camera->view_proj);
	Shader::set_builtin(VIEW_POS, camera->position);

	ivec4 viewport = ivec4(0, 0, camera->resolution);
	GL::Viewport(viewport);
	GL::Scissor(viewport);

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
