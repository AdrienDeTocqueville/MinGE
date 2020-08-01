#include "Graphics/Graphics.h"
#include "Render/GLDriver.h"
#include "Render/RenderEngine.h"
#include "Render/CommandBuffer.h"

#include "Core/Serialization.h"
#include "Structures/EntityMapper.inl"

using namespace nlohmann;

void GraphicsSystem::save(SerializationContext &ctx) const
{
	json dump = indices.to_json();

	{
		json cam_dump = json::array();
		cam_dump.get_ptr<nlohmann::json::array_t*>()->reserve(cameras.size);
		for (uint32_t i = 1; i <= cameras.size; i++)
		{
			json cam = json::object();

			cam["near_plane"] = cameras.get<0>()[i].near_plane;
			cam["far_plane"] = cameras.get<0>()[i].far_plane;
			cam["clear_color"] = ::to_json(cameras.get<0>()[i].clear_color);
			cam["fov"] = cameras.get<0>()[i].fov;
			cam["size_scale"] = ::to_json(cameras.get<0>()[i].size_scale);
			cam["ortho"] = cameras.get<0>()[i].ortho;

			cam["depth_texture"] = cameras.get<0>()[i].depth_texture.uint();
			cam["color_texture"] = cameras.get<0>()[i].color_texture.uint();

			cam_dump.push_back(cam);
		}

		dump["cameras"].swap(cam_dump);
	}

	{
		json renderer_dump = json::array();
		renderer_dump.get_ptr<nlohmann::json::array_t*>()->reserve(renderers.size);
		for (uint32_t i = 1; i <= renderers.size; i++)
		{
			json renderer = json::object();
			json materials = json::array();

			for (uint32_t s = renderers.get<0>()[i].first_submesh; s < renderers.get<0>()[i].last_submesh; s++)
				materials.push_back(submeshes[s].material);

			renderer["mesh"] = renderers.get<0>()[i].mesh.uint();
			renderer["materials"] = materials;

			renderer_dump.push_back(renderer);
		}

		dump["renderers"].swap(renderer_dump);
	}

	{
		json light_dump = json::array();
		light_dump.get_ptr<nlohmann::json::array_t*>()->reserve(point_lights.size);
		for (uint32_t i = 1; i <= point_lights.size; i++)
		{
			json light = json::object();

			light["color"] = ::to_json(point_lights.get<0>()[i].color);
			light["radius"] = point_lights.get<0>()[i].radius;

			light_dump.push_back(light);
		}

		dump["point_lights"].swap(light_dump);
	}

	ctx.set_dependencies(transforms);
	ctx.swap_data(dump);
}

GraphicsSystem::GraphicsSystem(const SerializationContext &ctx):
	indices_capacity(0), keys_capacity(0),
	draw_order_indices(NULL), renderer_keys(NULL),
	transforms((TransformSystem*)ctx.get_dependency(0)),
	indices(ctx["indices"])
{
	auto cam_it = ctx["cameras"].begin();
	for (uint32_t i = 1; i < indices.size; i++)
	{
		uint32_t comp = indices.get<0>(i);
		if (comp == 0) continue;

		indices.map<0>(i, 0);
		auto cam = cam_it.value();
		resize_rt(init_camera(Entity::get(i), cam["fov"], cam["near_plane"], cam["far_plane"],
			cam["ortho"], ::to_vec2(cam["size_scale"]), ::to_vec4(cam["clear_color"]),
			Texture::get(cam["color_texture"]), Texture::get(cam["depth_texture"])));
		++cam_it;
	}

	auto renderer_it = ctx["renderers"].begin();
	for (uint32_t i = 1; i < indices.size; i++)
	{
		uint32_t comp = indices.get<1>(i);
		if (comp == 0) continue;

		indices.map<1>(i, 0);
		auto renderer = renderer_it.value();
		add_renderer(Entity::get(i), Mesh::get(renderer["mesh"]));
		++renderer_it;
	}

	auto light_it = ctx["point_lights"].begin();
	for (uint32_t i = 1; i < indices.size; i++)
	{
		uint32_t comp = indices.get<2>(i);
		if (comp == 0) continue;

		indices.map<2>(i, 0);
		auto light = light_it.value();
		add_point_light(Entity::get(i), ::to_vec3(light["color"]), light["radius"]);
		++light_it;
	}

	RenderEngine::add_buffer(&cmd_buffer);

	global.buffer = GL::GenBuffer();
	objects.buffer = GL::GenBuffer();
}
