#include "Profiler/profiler.h"
#include "Graphics/Graphics.h"
#include "Graphics/CommandBuffer.h"
#include "IO/Input.h"

Camera GraphicsSystem::add_camera(Entity entity, float FOV, float zNear, float zFar,
	bool orthographic, vec4 viewport, vec3 clear_color, unsigned clear_flags)
{
	uint32_t i = cameras.add();
	indices.map<0>(entity, i);


	camera_t *cam = cameras.get<0>() + i;

	cam->fov	= FOV;
	cam->zNear	= zNear;
	cam->zFar	= zFar;
	cam->ortho	= orthographic;
	cam->entity	= entity;
	cam->ss_viewport= viewport;


	camera_data_t *cam_data = cameras.get<1>() + i;

	vec2 ws = Input::window_size();
	cam_data->viewport = ivec4(viewport.x * ws.x,
		viewport.y * ws.y,
		viewport.z * ws.x,
		viewport.w * ws.y
	);

	cam_data->clear_color	= vec4(clear_color, 0.0f);
	cam_data->clear_flags	= clear_flags;


	if (orthographic)
	{
		float half_width = FOV * 0.5f;
		float half_height = half_width * cam_data->viewport.w / (float)cam_data->viewport.z;
		cam->projection = ortho(-half_width, half_width, -half_height, half_height, zNear, zFar);
	}
	else
	{
		float aspect_ratio = (float)cam_data->viewport.z / (float)cam_data->viewport.w;
		cam->projection = perspective(FOV, aspect_ratio, zNear, zFar);
	}

	return i; // TODO: handle generation
}

Renderer GraphicsSystem::add_renderer(Entity entity, Mesh mesh)
{
	uint32_t i = renderers.add();
	indices.map<1>(entity, i);

	submeshes_t subs = Mesh::meshes.get<0>()[mesh.id()];
	uint32_t first = submeshes.add(subs.count);

	for (uint32_t s = 0; s < subs.count; s++)
	{
		submeshes[i].submesh = Mesh::submeshes[subs.first + s];
		submeshes[i].material = 2; // TODO: default material
		submeshes[i].renderer = i;
	}

	renderers.get<0>()[i] = renderer_t {first, first + subs.count, entity, mesh};

	return i;
}

Light GraphicsSystem::add_point_light(Entity entity, vec3 color)
{
	uint32_t i = point_lights.size();
	indices.map<2>(entity, i);

	point_lights.emplace_back(point_light_t { color, entity });

	return i;
}
