#include "Profiler/profiler.h"
#include "Graphics/Graphics.h"
#include "Render/CommandBuffer.h"
#include "IO/Input.h"

Camera GraphicsSystem::add_camera(Entity entity, float fov, float near_plane, float far_plane,
	bool orthographic, vec4 viewport, vec3 clear_color, unsigned clear_flags)
{
	uint32_t i = cameras.add();
	indices.map<0>(entity, i);


	camera_t *cam = cameras.get<0>() + i;

	cam->fov	= fov;
	cam->near_plane	= near_plane;
	cam->far_plane	= far_plane;
	cam->ortho	= orthographic;
	cam->entity	= entity;
	cam->ss_viewport= viewport;


	camera_data_t *cam_data = cameras.get<1>() + i;

	cam_data->fbo		= 0;
	cam_data->clear_flags	= clear_flags;
	cam_data->clear_color	= vec4(clear_color, 0.0f);

	vec2 ws = Input::window_size();
	cam_data->viewport = ivec4(viewport.x * ws.x,
		viewport.y * ws.y,
		viewport.z * ws.x,
		viewport.w * ws.y
	);

	update_projection(i);
	return {i, 0, *this};
}

Renderer GraphicsSystem::add_renderer(Entity entity, Mesh mesh)
{
	uint32_t i = renderers.add();
	indices.map<1>(entity, i);

	renderers.get<0>()[i].entity = entity;
	renderers.get<0>()[i].mesh = mesh;

	update_submeshes(i, false);

	return {i, 0, *this};
}

Light GraphicsSystem::add_point_light(Entity entity, vec3 color)
{
	uint32_t i = point_lights.size();
	indices.map<2>(entity, i + 1);

	point_lights.emplace_back(point_light_t { color, entity });

	return {i, 0, *this};
}


void GraphicsSystem::update_projection(uint32_t i)
{
	camera_t *cam = cameras.get<0>() + i;
	camera_data_t *cam_data = cameras.get<1>() + i;

	if (cam->ortho)
	{
		float half_width = cam->fov * 0.5f;
		float half_height = half_width * cam_data->viewport.w / (float)cam_data->viewport.z;
		cam->projection = ortho(-half_width, half_width, -half_height, half_height, cam->near_plane, cam->far_plane);
	}
	else
	{
		float aspect_ratio = (float)cam_data->viewport.z / (float)cam_data->viewport.w;
		cam->projection = perspective(glm::radians(cam->fov), aspect_ratio,
			cam->near_plane, cam->far_plane);
	}
}

void GraphicsSystem::update_submeshes(uint32_t i, bool remove_previous)
{
	renderer_t *r = renderers.get<0>() + i;

	if (remove_previous && r->first_submesh != submeshes.invalid_id())
		submeshes.remove(r->first_submesh, r->last_submesh - r->first_submesh);

	if (r->mesh != Mesh::none)
	{
		submeshes_t subs = Mesh::meshes.get<0>()[r->mesh.id()];
		uint32_t first = submeshes.add(subs.count);

		for (uint32_t s = 0; s < subs.count; s++)
		{
			submeshes[first+s].submesh = Mesh::submeshes[subs.first + s];
			submeshes[first+s].material = 2; // TODO: default material
			submeshes[first+s].renderer = i - 1;
		}

		r->first_submesh = first;
		r->last_submesh = first + subs.count;
	}
	else
		r->first_submesh = r->last_submesh = submeshes.invalid_id();
}
