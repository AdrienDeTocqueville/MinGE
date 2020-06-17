#include "Graphics/Graphics.h"
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

			cam["fov"] = cameras.get<0>()[i].fov;
			cam["near_plane"] = cameras.get<0>()[i].near_plane;
			cam["far_plane"] = cameras.get<0>()[i].far_plane;
			cam["viewport"] = ::to_json(cameras.get<0>()[i].ss_viewport);
			cam["ortho"] = cameras.get<0>()[i].ortho;
			cam["clear_color"] = ::to_json(cameras.get<1>()[i].clear_color);
			cam["clear_flags"] = cameras.get<1>()[i].clear_flags;

			cam_dump.push_back(cam);
		}

		dump["cameras"] = cam_dump;
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

		dump["renderers"] = renderer_dump;
	}

	ctx.set_dependencies(transforms);
	ctx.set_data(dump);
}

GraphicsSystem::GraphicsSystem(const SerializationContext &ctx):
	prev_index_count(0), prev_key_count(0), prev_renderer_count(0),
	draw_order_indices(NULL), renderer_keys(NULL), matrices(NULL),
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
		add_camera(Entity::get(i), cam["fov"], cam["near_plane"], cam["far_plane"], cam["ortho"],
			::to_vec4(cam["viewport"]), ::to_vec3(cam["clear_color"]), cam["clear_flags"]);
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
	
	cmd_buffer = RenderEngine::create_cmd_buffer();
}
