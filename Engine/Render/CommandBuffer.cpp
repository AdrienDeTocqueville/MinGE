#include "Profiler/profiler.h"

#include "Render/CommandBuffer.h"
#include "Render/Shaders/Shader.inl"
#include "Render/Shaders/Material.inl"

extern GLuint empty_vao;

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
uint32_t i = 0;
while (i < size)
{
Type cmd = consume<Type>(i);
switch (cmd)
{
case DrawBatch:
{
	draw_batch_t &batch = consume<draw_batch_t>(i);

	for (uint32_t c = 0; c < batch.count; c++)
	{
		uint32_t s = batch.sorted_indices[c];
		submesh_data_t *data = batch.submeshes + s;
		material_t *material = Material::materials.get<0>(data->material);
		if (!material->has_pass(batch.pass))
			continue;

		Shader::set_builtin("MATRIX_M", batch.matrices[data->renderer]);
		Shader::set_builtin("MATRIX_N", batch.matrices[data->renderer]);

		material->bind(batch.pass);

		{ MICROPROFILE_SCOPEGPUI("DrawElements", -1);
		GL::BindVertexArray(data->submesh.vao);
		glCheck(glDrawElements(data->submesh.mode, data->submesh.count, GL_UNSIGNED_SHORT,
					(void*)(uint64_t)data->submesh.offset));
		}
	}
	break;
}

case SetFramebuffer:
{
	set_framebuffer_t &setup = consume<set_framebuffer_t>(i);

	GL::BindFramebuffer(setup.fbo);
	GL::ClearColor(setup.clear_color);
	if (setup.clear_depth)
	{
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
	else
	{
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		//glDepthMask(GL_FALSE);
		//glDepthFunc(GL_EQUAL);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	break;
}

case SetupCamera:
{
	camera_data_t *&camera = consume<camera_data_t*>(i);

	GL::Enable(GL::CullFace);
	GL::Enable(GL::DepthTest);
	GL::Enable(GL::ScissorTest);
	GL::Disable(GL::Blend);

	Shader::set_builtin("MATRIX_VP", camera->view_proj);
	Shader::set_builtin("VIEW_POS", camera->position);

	ivec4 viewport = ivec4(0, 0, camera->resolution);
	GL::Viewport(viewport);
	GL::Scissor(viewport);

	break;
}

case FullscreenPass:
{
	fullscreen_pass_t &setup = consume<fullscreen_pass_t>(i);

	GL::BindFramebuffer(setup.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	GL::Viewport(setup.viewport);
	GL::Scissor(setup.viewport);

	material_t *material = Material::materials.get<0>(setup.material);
	if (!material->has_pass(RenderPass::Forward))
		break;

	material->bind(RenderPass::Forward);

	{ MICROPROFILE_SCOPEGPUI("FullscreenPass", -1);
	GL::BindVertexArray(empty_vao);
	glCheck(glDrawArrays(GL_TRIANGLES, 0, 6));
	}

	break;
}
}
}
size = 0;
}
