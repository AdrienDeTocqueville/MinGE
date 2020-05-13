#include "Profiler/profiler.h"

#include "Graphics/CommandBuffer.h"
#include "Graphics/Shaders/Shader.inl"
#include "Graphics/Shaders/Material.inl"

void cmd_buffer_t::draw_batch(submesh_data_t *submeshes, mat4 *matrices, uint32_t *sorted_indices,
		RenderPass::Type pass, uint32_t count)
{
	draw_batch_t cmd = {
		submeshes, matrices, sorted_indices,
		pass, count
	};
	store(DrawBatch, cmd);
}

void cmd_buffer_t::setup_camera(camera_data_t *camera)
{
	store(SetupCamera, camera);
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

		Shader::set_builtin("MATRIX_M", batch.matrices[data->renderer]);
		Shader::set_builtin("MATRIX_N", batch.matrices[data->renderer]);

		material_t *material = Material::materials.get<0>(data->material);
		material->bind(batch.pass);

		{ MICROPROFILE_SCOPEGPUI("DrawElements", -1);
		GL::BindVertexArray(data->submesh.vao);
		glCheck(glDrawElements(data->submesh.mode, data->submesh.count, GL_UNSIGNED_SHORT,
					(void*)(uint64_t)data->submesh.offset));
		}
	}
	break;
}

case SetupCamera:
{
	camera_data_t *&camera = consume<camera_data_t*>(i);

	GL::Enable(GL::CullFace);
	GL::Enable(GL::DepthTest);
	GL::Enable(GL::ScissorTest);

	GL::Viewport(camera->viewport);
	GL::Scissor (camera->viewport);
	GL::ClearColor(camera->clear_color);

	Shader::set_builtin("MATRIX_VP", camera->view_proj);
	Shader::set_builtin("VIEW_POS", camera->position);

	glClear(camera->clear_flags);
	break;
}
}
}
size = 0;
}
