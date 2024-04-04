#include "Graphics/Graphics.h"
#include "Render/GLDriver.h"
#include "Render/RenderEngine.h"
#include "Render/CommandBuffer.h"

#include "IO/DataModel.h"
#include "Structures/EntityMapper.inl"

void GraphicsSystem::transfer(Entity e, DataModel *model)
{
	uint8_t state = 0;
	if (model->is_writing())
	{
		if (has_camera(e))   state |= 1 << 0;
		if (has_renderer(e)) state |= 1 << 1;
		if (has_light(e))    state |= 1 << 2;
	}
	model->transfer("s", state);
	if (model->is_reading())
	{
		if (state & (1 << 0)) add_camera(e);
		if (state & (1 << 1)) add_renderer(e, Mesh());
		if (state & (1 << 2)) add_point_light(e);
	}

	if (has_camera(e))
	{
		camera_t &camera = cameras.get<0>()[indices.get<0>(e.id())];

		model->transfer("c.near_plane", camera.near_plane);
		model->transfer("c.far_plane", camera.far_plane);
		model->transfer("c.clear_color", camera.clear_color);
		model->transfer("c.fov", camera.fov);
		model->transfer("c.size_scale", camera.size_scale);
		model->transfer("c.ortho", camera.ortho);
		model->transfer("c.depth_texture", camera.depth_texture, Texture::type);
		model->transfer("c.color_texture", camera.color_texture, Texture::type);
	}

	if (has_renderer(e))
	{
		renderer_t &renderer = renderers.get<0>()[indices.get<1>(e.id())];

		model->transfer("r.mesh", renderer.mesh, Mesh::type);
		//for (uint32_t s = renderers.get<0>()[i].first_submesh; s < renderers.get<0>()[i].last_submesh; s++)
		//	materials.push_back(submeshes[s].material);
	}

	if (has_light(e))
	{
		point_light_t &light = point_lights.get<0>()[indices.get<2>(e.id())];

		model->transfer("l.color", light.color);
		model->transfer("l.radius", light.radius);
		model->transfer("l.intensity", light.intensity);
	}
}
