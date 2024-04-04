#pragma once

// GLTF importer

#include <fstream>

#include "Core/Engine.h"
#include "Core/AssetDatabase.h"

#include "Core/Entity.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Texture/Texture.h"
#include "Render/Shader/Material.h"

#include "Transform/Transform.h"
#include "Graphics/Graphics.h"

#include "IO/DataModel.h"
#include "IO/json.hpp"
#include "Math/glm.h"
#include "Utility/Log.h"

using namespace nlohmann;

static inline float make_float(const json &n, const char *prop, const float &def)
{
	if (!n.contains(prop)) return def;
	return n[prop].get<float>();
}

static inline vec3 make_vec3(const json &n, const char *prop, const vec3 &def)
{
	if (!n.contains(prop)) return def;
	json x = n[prop];
	return vec3(x[0].get<float>(), x[1].get<float>(), x[2].get<float>());
}

static inline quat make_quat(const json &n, const char *prop, const quat &def)
{
	if (!n.contains(prop)) return def;
	json x = n[prop];
	return quat(x[3].get<float>(),
		x[0].get<float>(),
		x[1].get<float>(),
		x[2].get<float>());
}

inline uint8_t decode_base64_char(char c) {
	static const uint8_t tableDecodeBase64[128] = {
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 0,  0,  0,  63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  64, 0,  0,
		0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  0,
		0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0,  0,  0,  0,  0
	};
	return tableDecodeBase64[size_t(c)];
}
static uint8_t *decode_base64_uri(int length, char const* uri)
{
	if (strncmp(uri, "data:", 5))
		return NULL;
	uri = strchr(uri + 5, ';') + 1;

	if (strncmp(uri, "base64", 6))
		return NULL;
	uri += 7;

	int size = length * 4 / 3, i = 0, j = 0;
	uint8_t *out = new uint8_t[length];

	for (; i + 4 < size; i += 4)
	{
		uint8_t b0 = decode_base64_char(uri[i + 0]);
		uint8_t b1 = decode_base64_char(uri[i + 1]);
		uint8_t b2 = decode_base64_char(uri[i + 2]);
		uint8_t b3 = decode_base64_char(uri[i + 3]);

		out[j++] = (uint8_t)((b0 << 2) | (b1 >> 4));
		out[j++] = (uint8_t)((b1 << 4) | (b2 >> 2));
		out[j++] = (uint8_t)((b2 << 6) | (b3 >> 0));
	}

	{
		uint8_t b0 = decode_base64_char(uri[i + 0]);
		uint8_t b1 = decode_base64_char(uri[i + 1]);
		uint8_t b2 = decode_base64_char(uri[i + 2]);
		uint8_t b3 = decode_base64_char(uri[i + 3]);

		out[j++] = (uint8_t)((b0 << 2) | (b1 >> 4));
		if (j < length)	out[j++] = (uint8_t)((b1 << 4) | (b2 >> 2));
		if (j < length)	out[j++] = (uint8_t)((b2 << 6) | (b3 >> 0));
	}

	return out;
}

static Mesh import_mesh(const json &mesh, const json &document, const std::vector<uint8_t*> &buffers)
{
	json prims = mesh["primitives"], accessors = document["accessors"];
	std::vector<submesh_t> submeshes; submeshes.reserve(prims.size());

	// 1 - Determine what data is needed and sort by materials
	bool has_pos = true, has_normal = true, has_uv = true, has_bones = true;
	uint32_t vertex_count = 0, index_count = 0;

	for (unsigned i = 0; i < prims.size(); i++)
	{
		json attribs = prims[i]["attributes"];

		if (!attribs.contains("POSITION"))   has_pos = false;
		if (!attribs.contains("NORMAL"))     has_normal = false;
		if (!attribs.contains("TEXCOORD_0")) has_uv = false;
		if (!attribs.contains("JOINTS_0"))   has_bones = false;
		if (!attribs.contains("WEIGHTS_0"))  has_bones = false;

		vertex_count += accessors[attribs["POSITION"].get<int>()]["count"].get<int>();
		index_count  += accessors[prims[i]["indices"].get<int>()]["count"].get<int>();
	}

	// 2 - Allocate necessary space
	int flags = 0;
	if (has_pos)    flags |= mesh_data_t::POINTS;
	if (has_normal) flags |= mesh_data_t::NORMALS;
	if (has_uv)     flags |= mesh_data_t::UVs;
	if (has_bones)  flags |= mesh_data_t::BONES;

	mesh_data_t data;
	data.init(vertex_count, index_count, (mesh_data_t::flags_t)flags);

	vertex_count = index_count = 0;
	for (unsigned i = 0; i < prims.size(); i++)
	{
		uint32_t count = 0, vertex_start = vertex_count;
		json attribs = prims[i]["attributes"];


		#define IMPORT(accessor, dest, dst_offset) do { \
			int view_id = accessors[accessor.get<int>()]["bufferView"].get<int>(); \
			json view = document["bufferViews"][view_id]; \
			\
			uint8_t* buf = buffers[view["buffer"].get<int>()]; \
			int offset = view["byteOffset"].get<int>(); \
			count = view["byteLength"].get<int>(); \
			\
			memcpy(dest + dst_offset, buf + offset, count); \
			count /= sizeof(*dest); \
		} while (0)

		// Read vertices
		if (flags & mesh_data_t::POINTS)  IMPORT(attribs["POSITION"], data.points, vertex_count);
		if (flags & mesh_data_t::NORMALS) IMPORT(attribs["NORMAL"], data.normals, vertex_count);
		if (flags & mesh_data_t::UVs)     IMPORT(attribs["TEXCOORD_0"], data.uvs, vertex_count);
		if (flags & mesh_data_t::BONES)
		{
			json joints_acc = accessors[attribs["JOINTS_0"].get<int>()];
			json weights_acc = accessors[attribs["WEIGHTS_0"].get<int>()];

			json joints_view = document["bufferViews"][joints_acc["bufferView"].get<int>()];
			json weights_view = document["bufferViews"][weights_acc["bufferView"].get<int>()];

			int joints_offset = joints_view["byteOffset"].get<int>();
			int weights_offset = weights_view["byteOffset"].get<int>();

			uint8_t* joints_buf = buffers[joints_view["buffer"].get<int>()] + joints_offset;
			uint8_t* weights_buf = buffers[weights_view["buffer"].get<int>()] + weights_offset;

			count = joints_acc["count"].get<int>(); // == weights_acc["count"]

			for (uint32_t j(0); j < count; j++)
			{
				uint16_t *joints = (uint16_t*)joints_buf + j * 4;
				float *weights = (float*)weights_buf + j * 4;

				data.bones[vertex_count + j].bones = uvec4(joints[0], joints[1], joints[2], joints[3]);
				data.bones[vertex_count + j].weights = vec4(weights[0], weights[1], weights[2], weights[3]);
			}
		}
		vertex_count += count;

		// Read indices
		IMPORT(prims[i]["indices"], data.indices, index_count);
		for (int j(0); j < count; j++)
			data.indices[index_count + j] += vertex_start;

		submeshes.push_back({GL_TRIANGLES, count, index_count, 0});
		index_count += count;

		#undef IMPORT
	}

	std::string name = mesh["name"].get<std::string>();
	Mesh mesh_asset = Mesh::create(name.c_str());

	mesh_asset.set_mesh_data(std::move(data), submeshes.data(), submeshes.size());

	return mesh_asset;
}


void scene_importer(import_context_t &ctx)
{
	std::ifstream file(ctx.path);
	if (!file)
	{
		Log::addf(Log::Error, "Failed to read '%s'.", ctx.path);
		return;
	}

	auto document = json::parse(file);

	// Asset data
	json SCENES = document["scenes"];
	json BUFFERS = document["buffers"];
	json MATERIALS = document["materials"];
	json MESHES = document["meshes"];
	json SKINS = document["skins"];
	json NODES = document["nodes"];
	json CAMERAS = document["cameras"];

	// Decode base64 buffers
	std::vector<uint8_t*> buffers;
	for (auto &buffer : BUFFERS)
	{
		buffers.push_back(decode_base64_uri(
			buffer["byteLength"].get<int>(),
			buffer["uri"].get_ptr<json::string_t*>()->c_str()
		));
		buffer.erase("uri");
	}

	// Import meshes
	std::vector<Mesh> meshes;
	meshes.reserve(MESHES.size());

	for (auto &mesh : MESHES)
	{
		Mesh m = import_mesh(mesh, document, buffers);
		ctx.register_object(Mesh::type, m);
		meshes.push_back(m);
	}

	// Import nodes
	std::vector<Entity> nodes;
	nodes.reserve(NODES.size());

	TransformSystem *transform_sys = NEW_SYSTEM(TransformSystem)();
	GraphicsSystem *graphics_sys = NEW_SYSTEM(GraphicsSystem)(transform_sys);

	for (auto &node : NODES)
	{
		Entity entity = Entity::create(node["name"].get<std::string>().c_str());
		nodes.push_back(entity);

		transform_sys->add(entity,
			make_vec3(node, "translation", vec3(0.0f)),
			make_quat(node, "rotation", quat(vec3(0))),
			make_vec3(node, "scale", vec3(1.0f))
		);

		// Read hierarchy
		if (node.contains("children"))
		{
			//for (auto &child : node["children"])
			//	transform_sys->set_parent(entity, nodes[child.get<int>()]);
		}

		// Read components
		if (node.contains("mesh"))
		{
			int mesh_id = node["mesh"].get<int>();

			Mesh mesh = meshes[mesh_id];
			Renderer renderer = graphics_sys->add_renderer(entity, mesh);

			/*
			json primitives = MESHES[mesh_id]["primitives"];
			for (size_t j(0); j < primitives.size(); j++)
			{
				if (primitives[j].contains("material"))
					renderer.set_material(j, materials[primitives[j]["material"].get<int>()]);

			}
			*/
		}
		/*
		if (node.contains("skin"))
		{
			int skin_id = node["skin"].get<int>();
			proto->insert<Animator>(skeletons[skin_id]);
		}
		*/
		if (node.contains("camera"))
		{
			json cam = CAMERAS[node["camera"].get<int>()];
			if (cam.contains("perspective"))
			{
				json perspective = cam["perspective"];
				graphics_sys->add_camera(entity,
					perspective["yfov"].get<float>() * 50.0f, // TODO: find real transformation
					perspective["znear"].get<float>(),
					perspective["zfar"].get<float>()
				);
			}
			else Log::add(Log::Error, "Unknown camera type");

		}
		if (node.contains("extensions"))
		{
			json light = node["extensions"]["KHR_lights_punctual"];
			if (!light.is_null())
			{
				json LIGHTS = document["extensions"]["KHR_lights_punctual"]["lights"];
				light = LIGHTS[light["light"].get<int>()];
				std::string type_str = light["type"].get<std::string>();
				vec3 color = make_vec3(light, "color", vec3(0.6f));

				if (type_str == "point")
					graphics_sys->add_point_light(entity, color);
				/*else if (type_str == "directional")
					graphics_sys->add_directional_light(entity, color);
				else if (type_str == "spot")
					graphics_sys->add_spot_light(entity, color);
				*/
			}
		}
	}

	// Import scenes
	for (auto &scene : SCENES)
	{
		std::string name = scene["name"].get<std::string>();

		DataModelWriter writer;
		for (auto &node : scene["nodes"])
		{
			Entity e = nodes[node.get<int>()];
			writer.transfer(&Entity::type, e);
		}

		Scene scene_asset = Scene::create(name.c_str());
		Scene::scene_t &data = Scene::scenes[scene_asset.id()];

		data.system_types.push_back(TransformSystem::type.name);
		data.system_types.push_back(GraphicsSystem::type.name);
		data.serialized_scene = writer.to_string();

		ctx.register_object(Scene::type, scene_asset);
	}

	// Free mem
	Engine::free_system(graphics_sys);
	Engine::free_system(transform_sys);
	for (size_t i(0); i < buffers.size(); i++)
		delete[] buffers[i];
}

const asset_importer_t Scene::importer = []() {
	asset_importer_t i = {};

	i.import = scene_importer;
	return i;
}();

