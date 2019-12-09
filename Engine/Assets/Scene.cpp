#include "Assets/Scene.h"

#include "MinGE.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <fstream>

#include "Utility/IO/json.hpp"
using json = nlohmann::json;

/*
static unsigned argmin_weight(vec4 weights)
{
	unsigned argmin = 0;
	float min_val = 1.0f; // Weights should be between 0 and 1
	for (unsigned k(0); k < 4; k++)
	{
		if (weights[k] == 0.0f)
			return k;
		if (weights[k] < min_val)
		{
			min_val = weights[k];
			argmin = k;
		}
		k++;
	}
	return argmin;
}
*/

static inline vec3 make_vec3(json &n, const vec3 &def)
{
	if (n.is_null()) return def;
	return vec3(n[0].get<float>(), n[1].get<float>(), n[2].get<float>());
}

static inline quat make_quat(json &n, const quat &def)
{
	if (n.is_null()) return def;
	return quat(n[0].get<float>(),
		n[1].get<float>(),
		n[2].get<float>(),
		n[3].get<float>());
}

inline char encode_base64_char(uint8_t b) {
	return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="[size_t(b)];
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

static MeshRef import_mesh(const json &mesh, const json &scene, const std::vector<uint8_t*> &buffers)
{
	json prims = mesh["primitives"], accessors = scene["accessors"];
	std::vector<Submesh> submeshes; submeshes.reserve(prims.size());

	// 1 - Determine what data is needed and sort by materials
	bool has_pos = true, has_normal = true, has_uv = true;
	uint32_t vertex_count = 0, index_count = 0;

	for (unsigned i = 0; i < prims.size(); i++)
	{
		json attribs = prims[i]["attributes"];

		if (!attribs.contains("POSITION"))   has_pos = false;
		if (!attribs.contains("NORMAL"))     has_normal = false;
		if (!attribs.contains("TEXCOORD_0")) has_uv = false;
		//if (!attribs.contains("WEIGHTS_0"))  has_bones = false;

		vertex_count += accessors[attribs["POSITION"].get<int>()]["count"].get<int>();
		index_count  += accessors[prims[i]["indices"].get<int>()]["count"].get<int>();
		//bone_count += submesh->mNumBones;
	}

	// 2 - Allocate necessary space
	MeshData::Flags flags = MeshData::Empty;
	if (has_pos)	flags = (MeshData::Flags)(flags | MeshData::Points);
	if (has_normal)	flags = (MeshData::Flags)(flags | MeshData::Normals);
	if (has_uv)	flags = (MeshData::Flags)(flags | MeshData::UVs);
	//if (bone_count)	flags = (MeshData::Flags)(flags | MeshData::Bones);

	MeshData data(vertex_count, index_count, flags);

	vertex_count = index_count = 0;
	for (unsigned i = 0; i < prims.size(); i++)
	{
		int count = 0, vertex_start = vertex_count;
		json attribs = prims[i]["attributes"];

		submeshes.push_back({GL_TRIANGLES, 0, index_count});


		#define IMPORT(accessor, dest, dst_offset) do { \
			int view_id = accessors[accessor.get<int>()]["bufferView"].get<int>(); \
			json view = scene["bufferViews"][view_id]; \
			\
			uint8_t* buf = buffers[view["buffer"].get<int>()]; \
			int offset = view["byteOffset"].get<int>(); \
			count = view["byteLength"].get<int>(); \
			\
			memcpy(dest + dst_offset, buf + offset, count); \
			count /= sizeof(*dest); \
		} while (0)

		// Read vertices
		if (flags & MeshData::Points)	IMPORT(attribs["POSITION"], data.points, vertex_count);
		if (flags & MeshData::Normals)	IMPORT(attribs["NORMAL"], data.normals, vertex_count);
		if (flags & MeshData::UVs)	IMPORT(attribs["TEXCOORD_0"], data.uvs, vertex_count);
		vertex_count += count;

		// Read indices
		IMPORT(prims[i]["indices"], data.indices, index_count);
		for (int j(0); j < count; j++)
			data.indices[index_count + j] += vertex_start;
		index_count += count;
		submeshes[i].count = count;

		#undef IMPORT
	}

	return MeshRef(new Mesh(std::move(data), submeshes));
}

static MaterialRef import_material(const json &material)
{
	auto m = Material::create("object");

	// Load PBR values
	auto pbr = material["pbrMetallicRoughness"];

	vec3 color = make_vec3(pbr["baseColorFactor"], vec3(0.8f));
	float metallic = pbr["metallicFactor"];
	float roughness = pbr["roughnessFactor"];

	m->set("color", color);
	m->set("metallic", metallic);
	m->set("roughness", roughness);

	// Load textures
	// ...

	return m;
}

/*
static bool has_offset(Blender::Object *obj)
{
	float *p = obj->loc;
	float *r = obj->rot;
	float *s = obj->size;

	return	(p[0] == p[1] == p[2] == 0.0f) &&
		(r[0] == r[1] == r[2] == 0.0f) &&
		(s[0] == s[1] == s[2] == 0.0f);
}

static Blender::Object *get_entity(Blender::Object *obj)
{
	while (obj->_pad0 == nullptr)
		obj = obj->parent;

	return obj;
}
*/

Scene::Scene(const std::string &file)
{
	std::string path = "Resources/" + file;
	std::string base = file.substr(0, file.find_last_of("/\\"));

	// Parse file
	std::ifstream src(path);
	json root;
	src >> root;

	// Scene data
	json scene = root["scenes"][root["scene"].get<int>()];
	name = scene["name"].get<std::string>();

	json BUFFERS = root["buffers"];
	json MATERIALS = root["materials"];
	json MESHES = root["meshes"];
	json NODES = root["nodes"];

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

	// Import materials
	std::vector<MaterialRef> materials_ref;
	materials.reserve(MATERIALS.size());
	materials_ref.reserve(MATERIALS.size());

	for (auto &material : MATERIALS)
	{
		MaterialRef m = import_material(material);

		materials_ref.push_back(m);
		materials.push_back(m);
	}

	// Import meshes
	std::vector<MeshRef> meshes_ref;
	meshes.reserve(MESHES.size());
	meshes_ref.reserve(MESHES.size());

	for (auto &mesh : MESHES)
	{
		MeshRef m = import_mesh(mesh, root, buffers);

		meshes_ref.push_back(m);
		meshes.push_back(m);
	}

	// Import nodes
	std::vector<Entity*> prototypes;
	prototypes.reserve(nodes.size());

	for (auto &node : NODES)
	{
		auto *proto = Entity::create(node["name"].get<std::string>(), true,
			make_vec3(node["translation"], vec3(0.0f)),
			make_quat(node["rotation"], quat(vec3(0))),
			make_vec3(node["scale"], vec3(1.0f))
		);
		prototypes.push_back(proto);

		// Read hierarchy
		if (node.contains("children"))
		{
			auto *tr = proto->find<Transform>();
			for (auto &child : node["children"])
				prototypes[child.get<int>()]->find<Transform>()->setParent(tr);
		}

		// Read components
		if (node.contains("mesh"))
		{
			int mesh_id = node["mesh"].get<int>();
			json primitives = MESHES[mesh_id]["primitives"];

			MeshRef mesh = meshes_ref[mesh_id];
			std::vector<MaterialRef> mesh_materials;

			for (int j(0); j < primitives.size(); j++)
			{
				if (primitives[j].contains("material"))
					mesh_materials.push_back(materials_ref[primitives[j]["material"].get<int>()]);
				else
					mesh_materials.push_back(Material::getDefault());
			}

			proto->insert<Graphic>(mesh, mesh_materials);
		}
		if (node.contains("camera"))
		{
			json CAMERAS = root["cameras"];
			json cam = CAMERAS[node["camera"].get<int>()];
			if (cam.contains("persepective"))
			{
				json perspective = root["perspective"];
				proto->insert<Camera>(
					perspective["yfov"].get<float>(),
					perspective["znear"].get<float>(),
					perspective["zfar"].get<float>()
				);
			}
		}
		if (node.contains("extensions"))
		{
			json LIGHTS = root["extensions"]["KHR_lights_punctual"]["lights"];
			json light = node["extensions"]["KHR_lights_punctual"];
			if (!light.is_null())
			{
				light = LIGHTS[light["light"].get<int>()];
				std::string type_str = light["type"].get<std::string>();
				vec3 color = make_vec3(light["color"], vec3(0.6f));

				Light::Type type = Light::Point;
				if (type_str == "directional") type = Light::Directional;
				else if (type_str == "point") type = Light::Point;
				else if (type_str == "spot") type = Light::Spot;
				else Error::add(USER_ERROR, "Unknown light type");

				proto->insert<Light>(type, color);
			}
		}
	}

	for (auto &node : scene["nodes"])
		nodes.push_back(prototypes[node.get<int>()]);


	for (int i(0); i < buffers.size(); i++)
		delete[] buffers[i];
	buffers.clear();
}

void Scene::instantiate()
{
	for (Entity *e : nodes)
		Entity::clone(e);
}
