#pragma once

#include <cstdint>

#include "Graphics/Mesh/Mesh.h"
#include "Graphics/RenderPass.h"

struct submesh_data_t
{
	submesh_t submesh;
	uint32_t material;
	uint32_t renderer;
};

struct cmd_buffer_t
{
	enum Type { DrawRenderers };

	cmd_buffer_t(): buffer(NULL), size(0), capacity(0) {}

	void draw_renderers(submesh_data_t *submeshes, struct Material *materials, mat4 *matrices,
			RenderPass::Type pass, uint32_t *sorted_indices, uint32_t count);

	uint8_t *buffer;
	uint32_t size, capacity;
};
