#include "Assets/Scene.h"

#include "MinGE.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <fstream>

#include "Utility/IO/json.hpp"
using json = nlohmann::json;


SceneRef Scene::create(const std::string &file)
{
	Scene *scene = new Scene();
	if (scene->load(file))
		return SceneRef(scene);

	delete scene;
	return SceneRef(nullptr);
}

Scene::~Scene()
{
	for (Entity *e : nodes)
		e->destroy();
}

void Scene::spawn()
{
	if (spawned) return;

	entities.reserve(nodes.size());
	for (Entity *e : roots)
		entities.push_back(Entity::clone(e));
}

void Scene::despawn()
{
	if (!spawned) return;

	for (Entity *e : entities)
		e->destroy();
	entities.clear();
}

Entity *Scene::find_prototype(const Tag &_tag)
{
	for (Entity* proto: nodes)
		if (proto->tag == _tag) return proto;

	return nullptr;
}

Entity *Scene::find_entity(const Tag &_tag)
{
	for (Entity* entity: entities)
		if (entity->tag == _tag) return entity;

	return nullptr;
}


// GLTF import stuff

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
	MeshData::Flags flags = MeshData::Empty;
	if (has_pos)	flags = (MeshData::Flags)(flags | MeshData::Points);
	if (has_normal)	flags = (MeshData::Flags)(flags | MeshData::Normals);
	if (has_uv)	flags = (MeshData::Flags)(flags | MeshData::UVs);
	if (has_bones)	flags = (MeshData::Flags)(flags | MeshData::Bones);

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
		if (flags & MeshData::Bones)
		{
			json joints_acc = accessors[attribs["JOINTS_0"].get<int>()];
			json weights_acc = accessors[attribs["WEIGHTS_0"].get<int>()];

			json joints_view = scene["bufferViews"][joints_acc["bufferView"].get<int>()];
			json weights_view = scene["bufferViews"][weights_acc["bufferView"].get<int>()];

			int joints_offset = joints_view["byteOffset"].get<int>();
			int weights_offset = weights_view["byteOffset"].get<int>();

			uint8_t* joints_buf = buffers[joints_view["buffer"].get<int>()] + joints_offset;
			uint8_t* weights_buf = buffers[weights_view["buffer"].get<int>()] + weights_offset;

			count = joints_acc["count"].get<int>(); // == weights_acc["count"]

			for (int j(0); j < count; j++)
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
		index_count += count;
		submeshes[i].count = count;

		#undef IMPORT
	}

	return MeshRef(new Mesh(std::move(data), submeshes));
}

static MaterialRef import_material(const json &material)
{
	auto m = Material::create("standard");

	// Load PBR values
	auto pbr = material["pbrMetallicRoughness"];

	vec3 color = make_vec3(pbr, "baseColorFactor", vec3(0.8f));
	float metallic = make_float(pbr, "metallicFactor", 0.0f);
	float roughness = make_float(pbr, "roughnessFactor", 0.5f);

	m->set("color", color);
	m->set("metallic", metallic);
	m->set("roughness", roughness);

	// Load textures
	// TODO...

	return m;
}

static Skeleton import_skin(const json &skin, const json &scene, const std::vector<uint8_t*> &buffers)
{
	Skeleton skel;

	int view_id = skin["inverseBindMatrices"].get<int>();
	json view = scene["bufferViews"][view_id];

	int offset = view["byteOffset"].get<int>();
	mat4* buf = (mat4*) (buffers[view["buffer"].get<int>()] + offset);

	json joints = skin["joints"];
	skel.offsets.resize(joints.size());

	for (size_t i(0); i < joints.size(); i++)
	{
		int src = joints[i].get<int>();
		memcpy(skel.offsets.data() + src, buf + src, sizeof(mat4));
	}

	return skel;
}


bool Scene::load(const std::string &file)
{
	std::string path = "Assets/" + file;
	std::string base = file.substr(0, file.find_last_of("/\\"));

	// Parse file
	std::ifstream src(path);
	json root;
	try {
		src >> root;
	}
	catch (json::parse_error &e) {
		Error::add(Error::USER, "JSON parser error");
		return false;
	}

	// Scene data
	json scene = root["scenes"][root["scene"].get<int>()];
	//name = scene["name"].get<std::string>();

	json ROOTS = scene["nodes"];
	json BUFFERS = root["buffers"];
	json MATERIALS = root["materials"];
	json MESHES = root["meshes"];
	json SKINS = root["skins"];
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
	materials.reserve(MATERIALS.size());

	for (auto &material : MATERIALS)
		materials.push_back(import_material(material));

	// Import skeletons
	std::vector<Skeleton> skeletons;
	skeletons.reserve(MATERIALS.size());

	for (auto &skin : SKINS)
		skeletons.push_back(import_skin(skin, root, buffers));

	// Import meshes
	meshes.reserve(MESHES.size());

	for (auto &mesh : MESHES)
		meshes.push_back(import_mesh(mesh, root, buffers));

	// Import nodes
	nodes.reserve(NODES.size());

	for (auto &node : NODES)
	{
		auto *proto = Entity::create(node["name"].get<std::string>(), true,
			make_vec3(node, "translation", vec3(0.0f)),
			make_quat(node, "rotation", quat(vec3(0))),
			make_vec3(node, "scale", vec3(1.0f))
		);
		nodes.push_back(proto);

		// Read hierarchy
		if (node.contains("children"))
		{
			auto *tr = proto->find<Transform>();
			for (auto &child : node["children"])
				nodes[child.get<int>()]->find<Transform>()->setParent(tr);
		}

		// Read components
		if (node.contains("mesh"))
		{
			int mesh_id = node["mesh"].get<int>();
			json primitives = MESHES[mesh_id]["primitives"];

			MeshRef mesh = meshes[mesh_id];
			std::vector<MaterialRef> mesh_materials;

			for (size_t j(0); j < primitives.size(); j++)
			{
				if (primitives[j].contains("material"))
					mesh_materials.push_back(materials[primitives[j]["material"].get<int>()]);
				else
					mesh_materials.push_back(Material::getDefault());
			}

			proto->insert<Graphic>(mesh, mesh_materials);
		}
		if (node.contains("skin"))
		{
			int skin_id = node["skin"].get<int>();
			proto->insert<Animator>(skeletons[skin_id]);
		}
		if (node.contains("camera"))
		{
			json CAMERAS = root["cameras"];
			json cam = CAMERAS[node["camera"].get<int>()];
			if (cam.contains("perspective"))
			{
				json perspective = cam["perspective"];
				float fov = perspective["yfov"].get<float>();
				float ratio = make_float(perspective, "aspectRatio", 16.0f / 9.0f);

				proto->insert<Camera>(
					glm::degrees(atan(tan(0.5f * fov) * ratio)),
					perspective["znear"].get<float>(),
					perspective["zfar"].get<float>()
				);
			}
			else Error::add(Error::USER, "Unknown camera type");

		}
		if (node.contains("extensions"))
		{
			json light = node["extensions"]["KHR_lights_punctual"];
			if (!light.is_null())
			{
				json LIGHTS = root["extensions"]["KHR_lights_punctual"]["lights"];
				light = LIGHTS[light["light"].get<int>()];
				std::string type_str = light["type"].get<std::string>();
				vec3 color = make_vec3(light, "color", vec3(0.6f));

				Light::Type type = Light::Point;
				if (type_str == "directional") type = Light::Directional;
				else if (type_str == "point") type = Light::Point;
				else if (type_str == "spot") type = Light::Spot;
				else Error::add(Error::USER, "Unknown light type");

				proto->insert<Light>(type, color, true);
			}
		}
	}

	// Save root nodes
	roots.reserve(ROOTS.size());

	for (auto &root : ROOTS)
		roots.push_back(nodes[root.get<int>()]);


	for (size_t i(0); i < buffers.size(); i++)
		delete[] buffers[i];
	buffers.clear();

	return true;
}
